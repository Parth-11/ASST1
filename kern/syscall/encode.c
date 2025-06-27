/* Supplied by UNSW as part of the Operating Systems course. */

#include <types.h>
#include <thread.h>
#include <syscall.h>
#include <kern/encoder161.h>
#include <kern/errno.h>
#include <encode_dev.h>
#include <synch.h>

static struct semaphore **lockbox_sems = NULL;
static struct lock *dev_lock = NULL;
static bool init = false;

/* Call this operation if a checksum fails. */
static inline void failed_checksum (void)
{
	kprintf("Checksum failed. Stopping thread to ensure security.\n");
	thread_exit();
}

// implementing dummy functions for kernel_encode_call && encode_system_setup
static int help_encode(int op_id,int arg,int box_id1,int box_id2,int *retval);

int kernel_encode_call(int op_num,unsigned int arg,int box_id1,int box_id2,int *result){
	
	int err;

	if(!init){
		err = encode_system_setup();
		if(err)
			return err;
	}

	err = help_encode(op_num,arg,box_id1,box_id2,result);
	if(err)
		return err;

	return 0;
}

static void acquire_lockbox(int box1,int box2){
	if(box1 == box2)
		P(lockbox_sems[box1]);
	else if(box1 < box2){
		P(lockbox_sems[box1]);
		P(lockbox_sems[box2]);
	}
	else{
		P(lockbox_sems[box2]);
		P(lockbox_sems[box1]);
	}
}

static void release_lockbox(int box1,int box2){
	if(box1 == box2)
		V(lockbox_sems[box1]);
	else if(box1 < box2){
		V(lockbox_sems[box2]);
		V(lockbox_sems[box1]);
	}
	else{
		V(lockbox_sems[box1]);
		V(lockbox_sems[box2]);
	}
}

static int help_encode(int op_id,int arg,int box_id1,int box_id2,int *retval){
	struct encode_device *dev = get_encode_device();
	if(dev == NULL)
		return ENODEV;
	int mode = dev->op_supported(op_id);
	if(mode == ENCODE_OP_NOT_SUPPORTED)
		return EINVAL;
	int result;
	if(mode == ENCODE_OP_ARG){
		if(box_id1!=-1 || box_id2 != -1)
			return EINVAL;
		
			lock_acquire(dev_lock);
			result = dev->encode_op(op_id,arg,NULL);
			*retval = result;
			lock_release(dev_lock);
			return;
	}

	int num_lockbox = dev->num_enc_boxes;

	if(box_id1<0 || box_id1>=num_lockbox || box_id2<0 || box_id2>=num_lockbox)
		return EINVAL;
	
	struct enc_boxes *boxes = kmalloc(sizeof(struct enc_boxes));

	if(boxes == NULL)
		return ENOMEM;
	
	boxes->box_id1 = box_id1;
	boxes->box_id2 = box_id2;

	acquire_lockbox(box_id1,box_id2);
	result = dev->encode_op(op_id,arg,boxes);
	*retval = result;
	release_lockbox(box_id1,box_id2);

	kfree(boxes);

	return 0;

}

int encode_system_setup(){

	struct encode_device *dev;

	if(init)
		return 0;
	
	dev = get_encode_device();
	if(dev == NULL)
		return ENODEV;

	int num_lockbox = dev->num_enc_boxes;
	lockbox_sems = kmalloc(num_lockbox * sizeof(struct semaphore *));
	
	if(lockbox_sems == NULL)
		return ENOMEM;
	
	for(int i=0;i<num_lockbox;i++){
		char name[32];
		snprintf(name,sizeof(name),"box_locks_%d",i);
		lockbox_sems[i] = sem_create(name,1);
		if(lockbox_sems[i] == NULL)
			return ENOMEM;	
	}

	dev_lock = lock_create("encoder_device_lock");
	if(dev_lock == NULL)
		return ENOMEM;

	init = true;

	return 0;
}

/* implement encode and checksum here */

int sys_encode(int op_id,int argument,int box_id1,int box_id2,int *retval){
	
	int err;

	if(!init){
		err = encode_system_setup();
		if(err)
			return err;
	}

	return help_encode(op_id,argument,box_id1,box_id2,retval);

}

int sys_checksum(int op_id,int argument){

	int err;
	if(!init){
		err = encode_system_setup();
		if(err)
			return err;
	}

	struct encode_device *device = get_encode_device();
	if(device == NULL)
		return ENODEV;
	
	lock_acquire(dev_lock);
	int result = device->checksum_op(op_id,argument);
	lock_release(dev_lock);

	if( result != 0){
		failed_checksum();
	}

	if(op_id == CHECKSUM_FINISHED && argument == 0){
		kprintf("Checksum verification succeeded\n");
	}

	return result;
}
