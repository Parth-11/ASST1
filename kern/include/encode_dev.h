/* Supplied by UNSW for assignment 1 of the OS course. */

#ifndef _ENCODE_DEV_H_
#define _ENCODE_DEV_H_

enum {
	ENCODE_OP_NOT_SUPPORTED = 0,
	ENCODE_OP_ARG = 1,
	ENCODE_OP_ARG_BOXES = 2,
};

struct enc_boxes {
	int box_id1;
	int box_id2;
};

struct encode_device {
	int num_enc_boxes;

	/* Returns ENCODE_OP_ARG or ENCODE_OP_ARG_BOXES if the op is supported. */
	int (* op_supported) (int op_num);

	/* Performs an encode op. The op must be supported. */
	unsigned int (* encode_op) (int op_num, unsigned int arg,
		struct enc_boxes *boxes);

	/* Does a checksum op, returning error codes on unsupported or failed ops. */
	int (* checksum_op) (int op_num, unsigned int arg);
};

extern struct encode_device * get_encode_device (void);

#endif


