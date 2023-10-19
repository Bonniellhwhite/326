#include <iostream>
#include <string>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/mman.h>
#include <sys/ipc.h>
#include <sys/wait.h>
#include <sys/msg.h>
#include <sys/shm.h>  // Do we use the teacher's provided one or this one?
#include <vector>


using namespace std; 

// argc (Argument Count), argv (Argument vector)
int main(int argc, char* argv[]){
	if (argc == 3){
		cout<< "Master begins execution" << endl;

		// Retrieving # of processes from argument 
		int num_processes = *argv[1] - '0';  

		// Retrieving Name from 3ed element of argument vector 
		string ms_name = argv[2];

		//Creating shared memory segment  (0666 means read write permissions for owner, group, all)
		int shm = shm_open(ms_name.c_str(), O_CREAT | O_RDWR,0666);
		ftruncate(shm, 4096);
		void* ptr = mmap(0,4096, PROT_READ | PROT_WRITE, MAP_SHARED,shm, 0);
		cout << "Master created a shared memory segment named " << ms_name << endl;

		// Vector to store child process IDs
        vector<pid_t> child_processes;

		for(int i = 0; i < num_processes; i++){
			pid_t child = fork();

            if (child < 0) {
                cerr << "Fork failed." << endl;
                return 1;
            } else if (child == 0) {
                // Child process
                const char* child_args[] = {"./slave", to_string(i + 1).c_str(), nullptr};
                execvp("./slave", const_cast<char* const*>(child_args));

                // If execvp fails
                cerr << "Exec failed." << endl;
                return 1;
            } else {
                // Parent process
                child_processes.push_back(child);
            }
        }

        // Parent waits for all children to terminate
        cout << "Master waits for all child processes to terminate" << endl;
        for (pid_t kid : child_processes) {
            int status;
            waitpid(kid, &status, 0);
        }

        // Print the updated content of shared memory
        cout << "Updated content of shared memory segment after access by child processes:" << endl;
        // Add code to print the content of shared memory

        // Remove shared memory segment
        int removed_shm = shm_unlink(ms_name.c_str());
        if (removed_shm == 0) {
            cout << "Master removed shared memory segment, and is exiting" << endl;
        } else {
            cerr << "Error removing shared memory segment" << endl;
        }
	}else{
		cout << "2 Arguments Required" << endl;
		return 0;
	}
	
}
