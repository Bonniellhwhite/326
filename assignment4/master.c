#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <sys/shm.h>
#include <sys/mman.h>
#include <semaphore.h>

#include "myShm.h"

int main(int argc, char *argv[]) {
    if (argc != 4) {
        fprintf(stderr, "Usage: %s <n_children> <sharedmemory> <semaphore>\n", argv[0]);
        return 1;
    }

    int n_children = atoi(argv[1]);
    const char *sharedmemory = argv[2];
    const char *semaphore = argv[3];

    // Create the shared memory
    int shm_fd = shm_open(sharedmemory, O_CREAT | O_RDWR, 0666);
    ftruncate(shm_fd, sizeof(struct CLASS));
    struct CLASS *shared_data = (struct CLASS *)mmap(0, sizeof(struct CLASS), PROT_READ | PROT_WRITE, MAP_SHARED, shm_fd, 0);


    shared_data->index = 0;
    // Create the semaphore
    sem_t *sem = sem_open(semaphore, O_CREAT, 0666, 1);

    printf("Master begins execution\n");
    printf("Master created a shared memory segment named %s\n", sharedmemory);
    printf("Master initializes index in the shared structure to zero\n");
    printf("Master created a semaphore named %s\n", semaphore);

    // Create the child process
    for (int i = 0; i < n_children; ++i) {
        pid_t pid = fork();
        if (pid == 0) {
            char child_index[10];
            sprintf(child_index, "%d", i + 1);

            execl("./slave", "slave", child_index, sharedmemory, semaphore, NULL);
            perror("execl");

            exit(EXIT_FAILURE);
        }
    }

    printf("Master created %d child processes to execute slave\n", n_children);

    // Wait for children processes
    for (int i = 0; i < n_children; ++i) {
        wait(NULL);
    }

    printf("Master received termination signals from all %d child processes\n", n_children);
    printf("Updated content of shared memory segment after access by child processes:\n");

    for (int i = 0; i < shared_data->index; ++i) {
        printf("%d ", shared_data->response[i]);
    }
    printf("\n");

    // Semaphore resource management
    sem_close(sem);
    sem_unlink(semaphore);
    munmap(shared_data, sizeof(struct CLASS));
    shm_unlink(sharedmemory);

    printf("Master removed the semaphore\n");
    printf("Master closed access to shared memory, removed shared memory segment, and is exiting\n");

    return 0;
}