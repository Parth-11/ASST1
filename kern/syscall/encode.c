/* Supplied by UNSW as part of the Operating Systems course. */

#include <types.h>
#include <thread.h>
#include <syscall.h>
#include <kern/encoder161.h>
#include <kern/errno.h>
#include <encode_dev.h>

/* Call this operation if a checksum fails. */
static inline void failed_checksum (void)
{
	kprintf("Checksum failed. Stopping thread to ensure security.\n");
	thread_exit();
}

// implementing dummy functions for kernel_encode_call && encode_system_setup
int kernel_encode_call(int op_num,unsigned int arg,int box_id1,int box_id2,int *result){
	(void)op_num;
	(void)arg;
	(void)box_id1;
	(void)box_id2;
	(void)result;

	return 0;
}

int encode_system_setup(){
	return 0;
}

/* implement encode and checksum here */

int sys_encode(int op_id,int argument,int box_id1,int box_id2,int *retval){
	struct encode_device *dev = get_encode_device();

	if(dev == NULL)
		return ENODEV;
	
		int mode = dev->op_supported(op_id);
		
		if (mode == ENCODE_OP_NOT_SUPPORTED)
			return EINVAL;
		
			int result;

			if(mode == ENCODE_OP_ARG){
				if(box_id1!=-1 || box_id2!=-1)
					return EINVAL;
				result = dev->encode_op(op_id,argument,NULL);
				*retval = result;
				return 0;
			}

			int num_boxes = dev->num_enc_boxes;

			if(box_id1<0 || box_id1>=num_boxes || box_id2<0 || box_id2>=num_boxes)
				return EINVAL;
			
			struct enc_boxes *boxes = kmalloc(sizeof(struct enc_boxes));
			
			if (boxes == NULL)
				return ENOMEM;
			
			boxes->box_id1 = box_id1;
			boxes->box_id2 = box_id2;

			result = dev->encode_op(op_id,argument,boxes);
			*retval = result;
			kfree(boxes);

			return 0;
}

int sys_checksum(int op_id,int argument){
	struct encode_device *device = get_encode_device();
	if(device == NULL)
		return ENODEV;
	
		int result = device->checksum_op(op_id,argument);

		if( result != 0){
			failed_checksum();
		}

		if(op_id == CHECKSUM_FINISHED && argument == 0){
			kprintf("Checksum verification succeeded\n");
		}

		return result;
}
