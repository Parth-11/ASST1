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

/* The mystery test. This test is supplied nearly working, but what it does
 * is mysterious. The assignment spec will explain more.
 */

extern int secret_message[];
char message2[2000];

static void
test(void)
{
	int i, j;
	unsigned int x;

	for (i = 0; secret_message[i] != 0; i++) {
		if (i >= 2000) {
			return;
		}
		x = secret_message[i];
		for (j = 0; j < 100; j++) {
			/* Un-comment these once encode and checksum exist. */
			x = encode(ENCODE_SHIFTS, x, -1, -1);
		}
		checksum(CHECKSUM_MYSTERY, x);
		message2[i] = ((char) x);
	}

	checksum(CHECKSUM_FINISHED, 0);
}

int
main(void)
{
	test();

	return 0;
}

