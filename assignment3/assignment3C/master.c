#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/mman.h>
#include <sys/wait.h>
#include "mySmh.h"

int main(int argc, char* argv[]) {
    if (argc == 3) {
        printf("Master begins execution\n");

        // Retrieving # of processes from argument
        int num_processes = atoi(argv[1]);  // Use atoi to convert string to integer

        // Retrieving Name from 3rd element of argument vector
        char* ms_name = argv[2];

        // Creating shared memory segment (0666 means read-write permissions for owner, group, all)
        int shm = shm_open(ms_name, O_CREAT | O_RDWR, 0666);
        ftruncate(shm, sizeof(CLASS));  // Configure size of memory segment based on the structure
        CLASS* sharedData = (CLASS*)mmap(0, sizeof(CLASS), PROT_READ | PROT_WRITE, MAP_SHARED, shm, 0);  // Map shared memory segment

        sharedData->index = 0;  // Initialize index to 0

        printf("Master created a shared memory segment named %s\n", ms_name);

        // Array to store child process IDs
        pid_t child_processes[num_processes];

        for (int i = 0; i < num_processes; i++) {
            pid_t child = fork();

            if (child < 0) {
                perror("Fork failed.");
                return 1;
            } else if (child == 0) {
                // Child process
                char child_index[10];
                snprintf(child_index, sizeof(child_index), "%d", i);

                execlp("./slave", "./slave", child_index, ms_name, NULL); // Execute slave process

                // If execlp fails
                perror("Exec failed");
                return 1;
            } else {
                child_processes[i] = child;
            }
        }

        printf("Master waits for all child processes to terminate\n");
        for (int i = 0; i < num_processes; i++) {
            int status;
            if (waitpid(child_processes[i], &status, 0) == -1) {
                perror("Waitpid failed");
                return 1;
            }
        }
        printf("Master received termination signals from all %d child processes\n", num_processes);

        // Print the updated content of shared memory
        printf("Updated content of shared memory segment after access by child processes:\n");
        printf(" --- content of shared memory --- \n");
        for (int i = 0; i < 10; ++i) {
            printf("%d ", sharedData->response[i]);
        }
        printf("\n");

        // Remove shared memory segment
        int removed_shm = shm_unlink(ms_name);
        if (removed_shm == 0) {
            printf("Master removed shared memory segment, and is exiting\n");
        } else {
            perror("Error removing shared memory segment");
            return 1;
        }
    } else {
        printf("Incorrect # of Arguments\n");
        return 1;
    }

    return 0;
}
