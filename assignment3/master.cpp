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
#include <mySmh.h>

using namespace std;

int main(int argc, char* argv[]) {
    if (argc == 3) {
        cout << "Master begins execution" << endl;

        // Retrieving # of processes from argument
        int num_processes = stoi(argv[1]);  // Use stoi to convert string to integer

        // Retrieving Name from 3rd element of argument vector
        string ms_name = argv[2];

        // Creating shared memory segment (0666 means read-write permissions for owner, group, all)
        int shm = shm_open(ms_name.c_str(), O_CREAT | O_RDWR, 0666);
        ftruncate(shm, 4096);                                                    // Configure size of memory segment
        void* ptr = mmap(0, 4096, PROT_READ | PROT_WRITE, MAP_SHARED, shm, 0);   // Map shared memory segment in address space of the proccess, aka shared memory base 

        int* index = static_cast<int*>(ptr);                                     //Initializing a pointer
        *index = 0;  // Initialize index to 0                                    // Setting starting pointer to 0 for memory segment writes 

        cout << "Master created a shared memory segment named " << ms_name << endl;

        // Vector to store child process IDs
        vector<pid_t> child_processes;

        for (int i = 0; i < num_processes; i++) {       // for every process
            pid_t child = fork();                       // Fork Parent aka Master 

            if (child < 0) {                            // if id is negative, failed 
                cerr << "Fork failed." << endl;
                return 1;
            } else if (child == 0) {                   // if success execute slave
                // Child process
                const char* child_args[] = {"./slave", to_string(i).c_str(), ms_name.c_str(), nullptr}; // creating an array of args, process number to cstr, and msname to cstr
                execvp("./slave", const_cast<char* const*>(child_args)); // changing process image to slave file, executing those instructions 

                // If execvp fails
                perror("Exec failed");
                return 1;
            } else {
                child_processes.push_back(child); // if everything was successful, add id to child process vector
            }
        }

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
        // Display the content of the shared memory
        const char* content = static_cast<const char*>(ptr);
        cout << "Shared Memory Content: " << content << endl;



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
