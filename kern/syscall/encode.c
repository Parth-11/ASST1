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

/* implement encode and checksum here */

