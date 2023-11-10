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
#include <sys/wait.h>
#include <vector>
#include <semaphore.h> 
#include "mySmh.h"

using namespace std;

int main(int argc, char* argv[]) {
    if (argc == 4) {
        cout << "Master begins execution" << endl;

        // Retrieving # of processes from argument
        int num_processes = stoi(argv[1]);  // Use stoi to convert string to integer

        // Retrieving Name from 3rd element of argument vector
        string ms_name = argv[2];

        // Retrieving Name from 4th element of argument vector 
        string semaphore_name = argv[3];
    
        
        // Creating shared memory segment (0666 means read-write permissions for owner, group, all)
        cout << "Master created a shared memory segment named " << ms_name << endl;
        int shm = shm_open(ms_name.c_str(), O_CREAT | O_RDWR, 0666);
        ftruncate(shm, sizeof(CLASS));  // Configure size of memory segment
        CLASS* sharedData = static_cast<CLASS*>(mmap(0, sizeof(CLASS), PROT_READ | PROT_WRITE, MAP_SHARED, shm, 0));  // Map shared memory segment
        cout << "Master initializes index in the shared structure to zero" << endl;
        sharedData->index = 0;

         // Create and initialize the semaphore
        cout << "Master created a semaphore named " << semaphore_name << endl;
        sem_t* mySemaphore = sem_open(semaphore_name.c_str(), O_CREAT | O_EXCL, 0666, 1);
        if (mySemaphore == SEM_FAILED) {
            perror("Semaphore creation failed");
            return 1;
        }

        // Vector to store child process IDs
        vector<pid_t> child_processes;

        for (int i = 0; i < num_processes; i++) {
            pid_t child = fork();

            if (child < 0) {
                cerr << "Fork failed." << endl;
                return 1;
            } else if (child == 0) {
                // Child process
                const char* child_args[] = {"./slave", to_string(i).c_str(), ms_name.c_str(), semaphore_name.c_str(), nullptr};
                execvp("./slave", const_cast<char* const*>(child_args));

                // If execvp fails
                perror("Exec failed");
                return 1;
            } else {
                child_processes.push_back(child);
            }
        }

        cout << "Master created " << num_processes << " child processes to execute slave" << endl;

        cout << "Master waits for all child processes to terminate" << endl;
        for (pid_t kid : child_processes) {
            int status;
            if (waitpid(kid, &status, 0) == -1) {
                perror("Waitpid failed");
                return 1;
            }
        }
        cout << "Master received termination signals from all " << num_processes << " child processes" << endl;

        // Print the updated content of shared memory
        cout << "Updated content of shared memory segment after access by child processes:" << endl;
        cout << " --- content of shared memory --- " << endl;
        for (int i = 0; i < 10; ++i) {
            cout << sharedData->response[i] << " ";
        }
        cout << endl;

        // Close and unlink the semaphore
        sem_close(mySemaphore);
        sem_unlink(semaphore_name.c_str());

        // Remove shared memory segment
        int removed_shm = shm_unlink(ms_name.c_str());
        if (removed_shm == 0) {
            cout << "Master removed shared memory segment, and is exiting" << endl;
        } else {
            perror("Error removing shared memory segment");
            return 1;
        }
    } else {
        cout << "Incorrect # of Arguments" << endl;
        return 1;
    }

    return 0;
}
