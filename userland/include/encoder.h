#ifndef _ENCODER_H_
#define _ENCODER_H_


/* to-do: write prototypes for the syscalls */
int encode(int op_id,int argument,int box_id1,int box_id2);
int checksum(int op_id,int argument);

#endif

