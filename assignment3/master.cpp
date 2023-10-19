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
        ftruncate(shm, sizeof(int));  // Assuming the shared memory is used for an integer (child number)

        void* ptr = mmap(0, sizeof(int), PROT_READ | PROT_WRITE, MAP_SHARED, shm, 0);
        int* index = static_cast<int*>(ptr);
        *index = 0;  // Initialize index to 0

        cout << "Master created a shared memory segment named " << ms_name << endl;

        // Vector to store child process IDs
        vector<pid_t> child_processes;

        for (int i = 0; i < num_processes; i++) {
            pid_t child = fork();

            if (child < 0) {
                cerr << "Fork failed." << endl;
                return 1;
            } else if (child == 0) {
                // Child process
                const char* child_args[] = {"./slave", to_string(i).c_str(), ms_name.c_str(), nullptr};
                execvp("./slave", const_cast<char* const*>(child_args));

                // If execvp fails
                perror("Exec failed");
                return 1;
            } else {
                // Parent process
                child_processes.push_back(child);
            }
        }

        // Parent waits for all children to terminate
        for (pid_t kid : child_processes) {
            int status;
            if (waitpid(kid, &status, 0) == -1) {
                perror("Waitpid failed");
                return 1;
            }
        }

        // Print the wait message
        cout << "Master waits for all child processes to terminate" << endl;
        cout << "Master received termination signals from all " << num_processes << " child processes" << endl;

        // Print the updated content of shared memory
        cout << "Updated content of shared memory segment after access by child processes:" << endl;
        cout << " --- content of shared memory --- " << *index << endl;

        // Remove shared memory segment
        int removed_shm = shm_unlink(ms_name.c_str());
        if (removed_shm == 0) {
            cout << "Master removed shared memory segment, and is exiting" << endl;
        } else {
            perror("Error removing shared memory segment");
            return 1;
        }
    } else {
        cout << "Usage: ./master <num_processes> <ms_name>" << endl;
        return 1;
    }

    return 0;
}
