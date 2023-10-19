#include <iostream>
#include <string>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/mman.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <sys/shm.h>

using namespace std;

int main(int argc, char* argv[]) {
    if (argc == 3) {
        int child_number = stoi(argv[1]);

        cout << "Slave begins execution" << endl;
        cout << "I am child number " << child_number << ", received shared memory name " << argv[2] << endl;

        // Open existing shared memory segment
        int shm = shm_open(argv[2], O_RDWR, 0666);
        if (shm == -1) {
            perror("Failed to open shared memory segment");
            return 1;
        }

        // Map shared memory into the address space of the process
        void* ptr = mmap(0, sizeof(int), PROT_READ | PROT_WRITE, MAP_SHARED, shm, 0);
        if (ptr == MAP_FAILED) {
            cerr << "Failed to map shared memory" << endl;
            close(shm);
            return 1;
        }

        // Access shared memory
        int* index = static_cast<int*>(ptr);
        int child_slot = (*index)++;  // Get the next available slot

        // Write child number to the shared memory
        int* child_number_ptr = static_cast<int*>(ptr) + 1 + child_slot;
        *child_number_ptr = child_number;

        // Print the message
        cout << "I have written my child number to slot " << child_slot << " and updated index to " << *index << endl;

        // Unmap shared memory
        munmap(ptr, sizeof(int));

        // Close shared memory
        close(shm);

        cout << "Child " << child_number << " closed access to shared memory and terminates." << endl;
    } else {
        cout << "Usage: ./slave <child_number> <ms_name>" << endl;
        return 1;
    }

    return 0;
}
