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

char test_string[48] = "The quick brown fox jumps over the lazy dog.";

/* your code here */
static void test(void){
	
	char encoded[48];
	int length = 44;

	for(int i=0;i<length;i++){
		int res = encode(ENCODE_ROT13,(int)test_string[i],-1,-1);
		if(res < 0)
			_exit(1);
		encoded[i] = (char)res;
	}

	encoded[length] = '\0';

	for(int i=0;i<length;i++){
		if(checksum(CHECKSUM_ROT13,(int)encoded[i])<0)
			_exit(1);
	}

	checksum(CHECKSUM_FINISHED,0);

	_exit(0);
}

int
main(void)
{
	/* and/or here */
	test();

	return 0;
}

