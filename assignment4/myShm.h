#ifndef MYSHM_H
#define MYSHM_H

#define MAX_CHILDREN 10

struct Slot {
    int child_number;
    int lucky_number;
};

struct CLASS {
    int index;
    Slot response[MAX_CHILDREN];
};

#endif // MYSHM_H
