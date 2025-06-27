/* Supplied by UNSW for assignment 1 of the OS course. */
/* Not for broader distribution or re-use. */

#include <types.h>
#include <kern/encoder161.h>
#include <encode_sys.h>
#include <encode_dev.h>
#include <lib.h>
#include <kern/errno.h>
#include <thread.h>
#include <synch.h>


struct encode_device *simulated_encoder;

static char
rot13_code(char x) {
	int v;
	if ('A' <= x && x <= 'Z') {
		v = x - 'A';
		x = 'A';
	}
	else if ('a' <= x && x <= 'z') {
		v = x - 'a';
		x = 'a';
	}
	else {
		return x;
	}
	v = v + 13;
	if (v >= 26) {
		v -= 26;
	}
	return x + ((char) v);
}

static unsigned int
rot13_int_code(unsigned int x) {
	if (x < 256) {
		return rot13_code(x);
	}
	else {
		return x;
	}
}

static unsigned int
shift_code(unsigned int x) {
	x = x ^ (x << 2);
	x = x ^ (x >> 7);
	return x;
}

/* Simulated encode boxes. A future version of the simulator might make these
 * values local to each instance of the simulated device rather than global.
 */
enum {
	NUM_SIM_BOXES = 16,
};
int encode_box_values[NUM_SIM_BOXES];

static unsigned int
box_encode(unsigned int x, struct enc_boxes *boxes) {

	KASSERT(boxes != NULL);
	KASSERT(boxes->box_id1 < NUM_SIM_BOXES);
	KASSERT(boxes->box_id2 < NUM_SIM_BOXES);

	/* This should be a fresh box. */
	if (encode_box_values[boxes->box_id2]) {
		return 0;
	}

	encode_box_values[boxes->box_id2] = x & 15;
	x = x >> 4;

	x = x ^ (encode_box_values[boxes->box_id1] << 24);
	encode_box_values[boxes->box_id1] = 0;

	return x;
}

/* Hint: this operation is an example of concurrency risks. */
static unsigned int
box_flip(unsigned int x, struct enc_boxes *boxes) {

	unsigned int y;

	KASSERT(boxes != NULL);
	KASSERT(boxes->box_id1 < NUM_SIM_BOXES);
	KASSERT(boxes->box_id2 < NUM_SIM_BOXES);

	y = x ^ encode_box_values[boxes->box_id2];

	encode_box_values[boxes->box_id1] = encode_box_values[boxes->box_id1] ^ y;

	thread_yield();

	return encode_box_values[boxes->box_id2];
}

/* Hint: this operation is an example of concurrency risks. */
static unsigned int
box_inc(unsigned int x, struct enc_boxes *boxes) {

	(void) x;
	unsigned int y1, y2;

	KASSERT(boxes != NULL);
	KASSERT(boxes->box_id1 < NUM_SIM_BOXES);
	KASSERT(boxes->box_id2 < NUM_SIM_BOXES);

	y1 = encode_box_values[boxes->box_id1] + 1;
	y2 = encode_box_values[boxes->box_id2] - 1;

	thread_yield();

	encode_box_values[boxes->box_id1] = y1;
	encode_box_values[boxes->box_id2] = y2;

	return 0;
}




static unsigned int
encode_op (int op_num, unsigned int arg, struct enc_boxes *boxes)
{
	switch(op_num) {
		case ENCODE_ROT13:

			KASSERT(boxes == NULL);

			return rot13_int_code(arg);

		case ENCODE_SHIFTS:

			KASSERT(boxes == NULL);

			return shift_code(arg);

		case ENCODE_BOX_OP_1:

			KASSERT(boxes != NULL);
			KASSERT(simulated_encoder != NULL);

			return box_encode(arg, boxes);

		case ENCODE_BOX_OP_2:

			KASSERT(boxes != NULL);
			KASSERT(simulated_encoder != NULL);

			return box_flip(arg, boxes);

		case ENCODE_BOX_INC:

			KASSERT(boxes != NULL);
			KASSERT(simulated_encoder != NULL);

			return box_inc(arg, boxes);


		default:

			KASSERT(! "Unreachable: unknown op num.");
	}
	return 0;
}

/* State of the checksum protocol. A future version of the simulator might
 * make these values local to each instance of the simulated device rather
 * than global. */
int checksum_protocol = 0;
int checksum_counter = 0;
unsigned int checksum_accumulator = 0;

const char * ref_string = "Gur dhvpx oebja sbk whzcf bire gur ynml qbt.";

static int
checksum_rot13_step (unsigned int arg)
{
	if (checksum_protocol != 0 && checksum_protocol != CHECKSUM_ROT13) {
		return EBUSY;
	}

	checksum_protocol = CHECKSUM_ROT13;

	if (arg != ((unsigned int) (ref_string[checksum_counter]))) {
		return EINVAL;
	}

	if (ref_string[checksum_counter] != 0) {
		checksum_counter ++;
	}

	return 0;
}

static int
checksum_boxes_step (unsigned int arg)
{
	if (checksum_protocol != 0 && checksum_protocol != CHECKSUM_BOXES) {
		return EBUSY;
	}

	checksum_protocol = CHECKSUM_BOXES;

	if (arg == 0 || arg >= 1000000) {
		return EINVAL;
	}

	if (checksum_counter < NUM_SIM_BOXES) {
		if (encode_box_values[checksum_counter]) {
			return EINVAL;
		}

		checksum_counter += 1;
	}

	return 0;
}

static int
checksum_mystery_step (unsigned int arg)
{
	if (checksum_protocol != 0 && checksum_protocol != CHECKSUM_MYSTERY) {
		return EBUSY;
	}

	checksum_protocol = CHECKSUM_MYSTERY;

	if (arg == ((unsigned int) ('\n'))) {
		checksum_counter ++;
		return 0;
	}

	if (arg < 32 || arg >= 128) {
		return EINVAL;
	}

	return 0;
}

static int
checksum_flip_step (unsigned int arg)
{
	int i;
	if (checksum_protocol != 0 && checksum_protocol != CHECKSUM_FLIP) {
		return EBUSY;
	}

	checksum_protocol = CHECKSUM_FLIP;

	checksum_accumulator = checksum_accumulator ^ arg;

	checksum_counter += 1;
	if (checksum_counter > 1) {
		return 0;
	}

	for (i = 0; i < NUM_SIM_BOXES; i ++) {
		checksum_accumulator = checksum_accumulator ^ encode_box_values[i];
	}
	return 0;
}

static int
checksum_finalise (void)
{
	switch(checksum_protocol) {
		case CHECKSUM_ROT13:
			if (ref_string[checksum_counter] != 0) {
				return EINVAL;
			}
			return 0;

		case CHECKSUM_BOXES:
			if (checksum_counter != 8) {
				return EINVAL;
			}
			return 0;

		case CHECKSUM_MYSTERY:
			if (checksum_counter == 7) {
				return 0;
			}
			else {
				return EINVAL;
			}

		case CHECKSUM_FLIP:
			if (checksum_counter > 0 && checksum_accumulator == 0) {
				return 0;
			}
			else {
				return EINVAL;
			}
	}

	return EINVAL;
}

static int
checksum_op (int op_num, unsigned int arg)
{
	int result;

	switch(op_num) {

		case CHECKSUM_FINISHED:
			kprintf("Attempting to finalise protocol %d.\n", checksum_protocol);
			result = checksum_finalise();
			if (result) {
				return result;
			}
			kprintf("Checksum completed (protocol %d)!\n", checksum_protocol);
			return 0;

		case CHECKSUM_RESET:
			checksum_protocol = 0;
			checksum_counter = 0;
			return 0;

		case CHECKSUM_ROT13:
			return checksum_rot13_step(arg);

		case CHECKSUM_BOXES:
			return checksum_boxes_step(arg);

		case CHECKSUM_MYSTERY:
			return checksum_mystery_step(arg);

		case CHECKSUM_FLIP:
			return checksum_flip_step(arg);
	}
	return EINVAL;
}

static int
op_supported (int op_num)
{
	switch(op_num) {
		case ENCODE_ROT13:
		case ENCODE_SHIFTS:

			return ENCODE_OP_ARG;

		case ENCODE_BOX_OP_1:
		case ENCODE_BOX_OP_2:
		case ENCODE_BOX_INC:

			return ENCODE_OP_ARG_BOXES;

		default:

			return ENCODE_OP_NOT_SUPPORTED;
	}
	return 0;
}

struct encode_device *
get_encode_device(void)
{
	struct encode_device *dev;

	if (simulated_encoder != NULL) {
		return simulated_encoder;
	}

	dev = kmalloc(sizeof(struct encode_device));
	if (dev == NULL) {
		return NULL;
	}

	dev->num_enc_boxes = 16;
	dev->op_supported = op_supported;
	dev->encode_op = encode_op;
	dev->checksum_op = checksum_op;

	simulated_encoder = dev;

	return dev;
}

/* Testing functionality. */

static struct lock * control_lock;
static struct semaphore * finished_threads;

enum {
	MAX_THREAD_COUNT = 16
};

static int thread_count;

unsigned int result_array[MAX_THREAD_COUNT];

static int encoder_is_setup = 0;

void
encode_setup_wrapper(void)
{
	int result;

	if ( encoder_is_setup) {
		return;
	}

	result = encode_system_setup();
	if (result) {
		panic("encode_system_setup failed with result %d\n", result);
	}
}

static void
box_reset (void)
{
	int i;

	for (i = 0; i < NUM_SIM_BOXES; i ++) {
		encode_box_values[i] = 0;
	}
}

static void
box_inc_test_thread(void *unused_ptr, unsigned long thread_id)
{
	(void) unused_ptr; /* Prevent compiler warnings. */
	int box1 = thread_id;
	int box2 = thread_id + 1;
	int i, result, y;

	if (box2 == thread_count) {
		box2 = 0;
	}

	/* Don't start up while the control lock is held. */
	lock_acquire(control_lock);
	lock_release(control_lock);

	for (i = 0; i < 250; i ++) {
		result = kernel_encode_call(ENCODE_BOX_INC, 0, box1, box2, &y);
		if (result) {
			kprintf("box_inc_test: thread %d failed: %d", (int)thread_id, result);
			break;
		}
		result = kernel_encode_call(ENCODE_BOX_INC, 0, box2, box1, &y);
		if (result) {
			kprintf("box_inc_test: thread %d failed: %d", (int)thread_id, result);
			break;
		}
	}

	V(finished_threads);
}

static void
start_inc_test_thread(int thread_id)
{
	int result;

	KASSERT(thread_id < MAX_THREAD_COUNT);

	result = thread_fork("inc test thread", NULL,
				box_inc_test_thread, NULL, thread_id);
	if(result) {
		panic("start_flip_test_thread: couldn't fork (%s)\n",
			strerror(result));
	}
}

static void
run_inc_test(void)
{
	int i;

	lock_acquire(control_lock);

	box_reset();

	for (i = 0; i < thread_count; i++) {
		start_inc_test_thread(i);
	}

	/* Set them going. */
	lock_release(control_lock);

	/* Wait for them to finish. */
	for (i = 0; i < thread_count; i++) {
		P(finished_threads);
	}

	for (i = 0; i < thread_count; i ++) {
		if (encode_box_values[i]) {
			kprintf("Box inc test failed in box %d.\n", i);
			return;
		}
	}
	kprintf("Box inc test succeeded!\n");
}

static void
box_flip_test_thread(void *unused_ptr, unsigned long thread_id)
{
	(void) unused_ptr; /* Prevent compiler warnings. */
	int box1 = thread_id;
	int box2 = thread_id + 1;
	unsigned int x;
	int i, result, y;

	if (box2 == thread_count) {
		box2 = 0;
	}

	/* Don't start up while the control lock is held. */
	lock_acquire(control_lock);
	lock_release(control_lock);

	result = 0;
	x = 0;
	for (i = 0; i < 250 && result == 0; i ++) {
		result = kernel_encode_call(ENCODE_BOX_OP_2, x ^ i, box1, box2, &y);
		x = y;
	}
	for (i = 0; i < 250 && result == 0; i ++) {
		result = kernel_encode_call(ENCODE_BOX_OP_2, x ^ i, box1, box2, &y);
		x = y;
	}

	result_array[thread_id] = x;

	V(finished_threads);
}

static void
start_flip_test_thread(int thread_id)
{
	int result;

	KASSERT(thread_id < MAX_THREAD_COUNT);

	result = thread_fork("flip test thread", NULL,
				box_flip_test_thread, NULL, thread_id);
	if(result) {
		panic("start_flip_test_thread: couldn't fork (%s)\n",
			strerror(result));
	}
}

static void
run_flip_test(void)
{
	int i;
	unsigned int x;

	lock_acquire(control_lock);

	box_reset();

	for (i = 0; i < thread_count; i++) {
		start_flip_test_thread(i);
	}

	/* Set them going. */
	lock_release(control_lock);

	/* Wait for them to finish. */
	for (i = 0; i < thread_count; i++) {
		P(finished_threads);
	}

	/* Accumulate. */
	for (i = 0, x = 0; i < thread_count; i ++) {
		x = x ^ result_array[i];
		x = x ^ encode_box_values[i];
	}

	if (x == 0) {
		kprintf("Box flip test succeeded!\n");
	}
	else {
		kprintf("Box flip test failed.\n");
	}
}

/* The main function of the encoder synch test framework. */
int
asst1_test(int nargs, char **args)
{
	int i;
	kprintf("asst1_test: starting up\n");

	/* Initialise synch primitives used in this testing framework */
	finished_threads = sem_create("asst1_finished_threads", 0);
	if(! finished_threads) {
		panic("asst1_test: couldn't create semaphore\n");
	}

	control_lock = lock_create("asst1_control_lock");
	if(control_lock == NULL ) {
		panic("asst1_test: couldn't create lock\n");
	}

	/* Ensure the student code is initialised. */
	encode_setup_wrapper();

	if (nargs == 1) {
		kprintf("asst1_test: nothing to test\n");
	}

	for (i = 1; i < nargs; i ++) {
		if (! strcmp(args[i], "inc")) {
			thread_count = MAX_THREAD_COUNT;
			run_inc_test();
		}
		else if (! strcmp(args[i], "flip")) {
			thread_count = MAX_THREAD_COUNT;
			run_flip_test();
		}
		else {
			kprintf("asst1_test: did not recognize '%s'\n", args[i]);
		}
	}

	/* Done! */
	sem_destroy(finished_threads);
	lock_destroy(control_lock);
	return 0;
}


