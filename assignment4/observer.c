/* observer.c
   compile with gcc, link with -lrt -lpthread
   run with ./observer /myShm /mySem
*/

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>
#include <sys/shm.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <semaphore.h>
#include <errno.h>

int main (int argc, char* argv[]) {
    const char *shmName = argv[1];   /* shared memory name from commandline */
    const char *semName = argv[2];   /* semaphore name from commandline */
    const int SIZE = 4096;

    int shm_fd, i;
    int *count_ptr;	/* pointer to shared variable count, 
                           need mutual exclusion for access */

    shm_fd = shm_open(shmName, O_CREAT|O_RDWR, 0666);
    if (shm_fd == -1) {
        printf("observer: Shared memory failed: %s\n", strerror(errno));
        exit(1);
    }

/* configure size of shared memory segment */
    ftruncate(shm_fd, SIZE);

/* map shared memory segment in the address space of the process */
    count_ptr = (int *) mmap(0,SIZE,PROT_READ|PROT_WRITE,MAP_SHARED,shm_fd,0);
    if (count_ptr == MAP_FAILED) {
        printf("observer: Map failed: %s\n", strerror(errno));
        exit(1);
    }

/*  Initialize shared count */
    *count_ptr = 10;
    printf("count initialized to %d\n", *count_ptr);

/* create a named semaphore for mutual exclusion */
    sem_t *mutex_sem = sem_open( semName, O_CREAT, 0660, 1);
    if (mutex_sem == SEM_FAILED) {
        printf("observer: sem_open failed: %s\n", strerror(errno));
        exit(1);
    }

/* lock mutex_sem for critical section */
    if (sem_wait(mutex_sem) == -1) {
        printf("observer: sem_wait failed: %s/n", strerror(errno));
        exit(1);
    }

/* critical section to access & increment count in shared memory */
    printf ("count before update = %d \n", *count_ptr);
    (*count_ptr)++;
    printf ("updated count = %d \n", *count_ptr);

/* exit critical section, release mutex_sem */
    if (sem_post(mutex_sem) == -1) {
        printf("observer: sem_post failed: %s\n", strerror(errno));
        exit(1);
    }

/* done with semaphore, close it & free up resources 
   allocated to it */
    if (sem_close(mutex_sem) == -1) {
        printf("observer: sem_close failed: %s\n", strerror(errno));
        exit(1);
    }

/* request to remove the named semaphore, but action won't 
   take place until all references to it have ended */
    if (sem_unlink(semName) == -1) {
        printf("observer: sem_unlink failed: %s\n", strerror(errno));
        exit(1);
    }

/* remove mapped memory segment from the address space */
    if (munmap(count_ptr,SIZE) == -1) {
        printf("observer: Unmap failed: %s\n", strerror(errno));
        exit(1);
    }

/* close shared memory segment */
    if (close(shm_fd) == -1) {
        printf("observer: Close failed: %s\n", strerror(errno));
        exit(1);
    }

/* remove shared memory segment from the file system */
    if (shm_unlink(shmName) == -1) {
        printf("observer: Error removing %s: %s\n", shmName, strerror(errno));
        exit(1);
    }

    return 0;
}
