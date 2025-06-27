/* Supplied by UNSW for assignment 1 of the OS course. */
/* Not for broader distribution or re-use. */

#include <types.h>
#include <kern/encoder161.h>
#include <synch.h>
#include <lib.h>

/* Prototypes of encode syscall layer functions. */

/* Set up, will be called prior to any other operations. */
int encode_system_setup (void);

/* Perform an encode operation from kernel mode, equivalent to a system call. */
int kernel_encode_call (int op_num, unsigned int arg, int box_id1, int box_id2, int *result);


/* Testing */

int asst1_test (int nargs, char **args);
void encode_setup_wrapper (void);


