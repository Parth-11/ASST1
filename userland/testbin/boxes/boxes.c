/*
 * Supplied by UNSW as part of the OS course assignment 1.
 *
 * Not intended for further distribution or re-use.
 */

#include <stdio.h>
#include <kern/encoder161.h>
#include <encoder.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

/* your code here */

int
main(void)
{

	/* and here */

	int input = 123456;
	int output = input;

	for(int i=0;i<8;i++){

		int box1 = i;
		int box2 = (i+1)%8;

		output = encode(ENCODE_BOX_OP_1,output,box1,box2);

		if(output <0)
			return 1;
	}

	if(input != output)
		return 1;
	
	for(int i=0;i<8;i++){
		if(checksum(CHECKSUM_BOXES,output)<0)
			return 1;
	}

	checksum(CHECKSUM_FINISHED,0);

	return 0;
}

