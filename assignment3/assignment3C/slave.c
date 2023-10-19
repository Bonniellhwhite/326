#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/mman.h>
#include "mySmh.h"

int main(int argc, char* argv[]) {
    if (argc == 3) {
        int child_number = atoi(argv[1]);

        printf("Slave begins execution\n");
        printf("I am child number %d, received shared memory name %s\n", child_number, argv[2]);

        // Open existing shared memory segment
        int shm = shm_open(argv[2], O_RDWR, 0666);
        if (shm == -1) {
            perror("Failed to open shared memory segment");
            return 1;
        }

        // Map shared memory into the address space of the process
        void* ptr = mmap(0, sizeof(CLASS), PROT_READ | PROT_WRITE, MAP_SHARED, shm, 0);
        if (ptr == MAP_FAILED) {
            fprintf(stderr, "Failed to map shared memory\n");
            close(shm);
            return 1;
        }

        // Access shared memory
        CLASS* sharedData = (CLASS*)ptr;
        int child_slot = sharedData->index++;  // Get the next available slot

        // Write child number to the shared memory
        sharedData->response[child_slot] = child_number;

        // Print the message
        printf("I have written my child number to slot %d and updated index to %d\n", child_slot, sharedData->index);

        // Unmap shared memory
        munmap(ptr, sizeof(CLASS));

        // Close shared memory
        close(shm);

        printf("Child %d closed access to shared memory and terminates.\n", child_number);
    } else {
        printf("Usage: ./slave <child_number> <ms_name>\n");
        return 1;
    }

    return 0;
}
