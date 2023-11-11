/* myShm.h */
/* Header file to be used with master.c and slave.c
*/

#ifndef MYSHM_H
#define MYSHM_H

struct CLASS {
int index; /* index to next available response slot */
int response[10]; /* each child writes its child number & lucky number here*/
};

#endif // MYSHM_H
