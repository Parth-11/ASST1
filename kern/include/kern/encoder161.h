/* Supplied by UNSW for assignment 1 of the OS course. */
/* Not for broader distribution or re-use. */

#ifndef _KERN_ENCODER161_H_
#define _KERN_ENCODER161_H_


/* Encode, decode and checksum operation numbers shared between kernel and
 * user code.
 */

enum {
	ENCODE_ROT13 = 1,
	ENCODE_SHIFTS = 3,
	ENCODE_BOX_OP_1 = 8,
	ENCODE_BOX_OP_2 = 9,
	ENCODE_BOX_INC = 11,
};

enum {
	CHECKSUM_FINISHED = 0,
	CHECKSUM_RESET = 1,
	CHECKSUM_ROT13 = 5,
	CHECKSUM_BOXES = 12,
	CHECKSUM_MYSTERY = 18,
	CHECKSUM_FLIP = 22,
	CHECKSUM_INC = 23,
};


#endif


