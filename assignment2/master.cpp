#include <iostream>
#include <sys/msg.h>
#include <sys/wait.h>
#include <unistd.h>
#include <vector>

using namespace std;

int main(int argc, char* argv[]) {

    
	if (argc != 2) {
		cerr << "Usage: ./master [numRecievers]" << endl;
		return 1;
	}

    // get msg id
	int msg_id = msgget(IPC_PRIVATE, 0666);

    // Creates an empty list for child process process ids
	vector<pid_t> child_processes;

    //stores the number of senders and receivers 
	int num_children = *argv[1] - '0';

    // Prints master process id and message queue id  
	cout << "Master, PID " << getpid()  << ", begins execution" << endl;
	cout << "Master acquired a message queue, id " << msg_id << endl;
	
	struct msqid_ds buf;	

    // for every process # defined fork and create a sender process 
	for (int i = 0; i < num_children; i++) {
		pid_t child = fork();

		// if child < 0, fork failed
		if (child < 0) {
			cerr << "Fork failed." << endl;
			return 1;
		}
		else if (child == 0) {
			const char* child_args[] = {"./sender", (to_string(msg_id)).c_str(), (to_string(i + 1)).c_str(), nullptr};
			pid_t child_process = execvp("./sender", const_cast<char* const*> (child_args));	
			if (child_process < 0) {
				cerr << "Exec failed." << endl;
				return 1;
			}
		}
		else {
			child_processes.push_back(child);
		}
	}
	cout << "Master created " << num_children << " child processes to serve as sender." << endl;

    // for every # of processes given, fork and create a reciever process 
	for (int i = 0; i < num_children; i++) {
		pid_t child = fork();

		if (child < 0) {
			cerr << "Fork failed." << endl;
			return 1;
		}
		else if (child == 0) {
			const char* child_args[] = {"./reciever", to_string(msg_id).c_str(), to_string(i + 1).c_str(), nullptr};
			pid_t child_process = execvp("./reciever", const_cast<char* const*> (child_args));	
			if (child_process < 0) {
				cerr << "Exec failed." << endl;
				return 1;
			}
			
		}
		else {
			child_processes.push_back(child);
		}
	}

	cout << "Master created " << num_children << " child processes to serve as reciever" << endl;

    // waits for all children to terminate 
	cout << "Master waits for all child processes to terminate" << endl;
	for (pid_t kid : child_processes) {
		int status;
		waitpid(kid, &status, 0);
	}	
	
    // Once terminated, report termination signals have been recieved
	msgctl(msg_id, IPC_STAT, &buf);
	int removed_msg = msgctl(msg_id, IPC_RMID, &buf);
	if (removed_msg >= 0) 
		cout << "Master recieved termination signals from all child processes, removed message queue, and terminates" << endl;
	return 0;

}