#include <iostream>
#include <string>
#include <vector>
#include <fcntl.h>
#include <unistd.h>
#include <sys/mman.h>
#include <semaphore.h>
#include <sys/wait.h> 
#include "myShm.h" // Ensure this is the correct header file name

using namespace std;

int main(int argc, char* argv[]) {
    if (argc != 4) {
        cerr << "Usage: " << argv[0] << " <num_processes> <shm_name> <sem_name>" << endl;
        return 1;
    }

    cout << "Master begins execution" << endl;

    int num_processes = stoi(argv[1]);
    string shm_name = argv[2];
    string sem_name = argv[3];

    int shm_fd = shm_open(shm_name.c_str(), O_CREAT | O_RDWR, 0666);
    if (shm_fd == -1) {
        perror("shm_open failed");
        return 1;
    }
    cout << "Master created a shared memory segment named " << shm_name <<endl;

    if (ftruncate(shm_fd, sizeof(CLASS)) == -1) {
        perror("ftruncate failed");
        close(shm_fd);
        return 1;
    }

    CLASS* shared_data = static_cast<CLASS*>(mmap(NULL, sizeof(CLASS), PROT_READ | PROT_WRITE, MAP_SHARED, shm_fd, 0));
    if (shared_data == MAP_FAILED) {
        perror("mmap failed");
        close(shm_fd);
        return 1;
    }

    shared_data->index = 0;
    cout << "Master initializes index in the shared structure to zero" << endl;

    sem_t* my_sem = sem_open(sem_name.c_str(), O_CREAT | O_EXCL, 0666, 1);
    if (my_sem == SEM_FAILED) {
        perror("sem_open failed");
        munmap(shared_data, sizeof(CLASS));
        close(shm_fd);
        return 1;
    }

    cout << "Master created a semaphore named "<< sem_name << endl;

    vector<pid_t> children;

    cout << "Master created " << num_processes << " child processes to execute slave"<< endl;
    for (int i = 0; i < num_processes; ++i) {
        pid_t pid = fork();
        if (pid == 0) {
            execlp("./slave", "slave", to_string(i).c_str(), shm_name.c_str(), sem_name.c_str(), (char *)NULL);
            perror("execlp failed");
            exit(1);
        } else if (pid > 0) {
            children.push_back(pid);
        } else {
            perror("fork failed");
            continue;
        }
    }

    cout << "Master waits for all child processes to terminate" << endl;
    for (pid_t child : children) {
        int status;
        waitpid(child, &status, 0);
    }
    cout << "Master received termination signals from all n child processes" << endl;

    cout << "Updated content of shared memory segment after access by child processes:" << endl;
    cout << "--- content of shared memory --- " << endl;
    for (int i = 0; i < 10; ++i) {                      // for every byte from mySmh.h, print contents
            cout << shared_data-> response[i] << " ";
        }
        cout << endl;
        

    if (sem_close(my_sem) == -1) {
        perror("sem_close failed");
    }
    if (sem_unlink(sem_name.c_str()) == -1) {
        perror("sem_unlink failed");
    }
    
    cout << "Master removed the semaphore" << endl;

    if (munmap(shared_data, sizeof(CLASS)) == -1) {
        perror("munmap failed");
    }
    if (close(shm_fd) == -1) {
        perror("close failed");
    }
    if (shm_unlink(shm_name.c_str()) == -1) {
        perror("shm_unlink failed");
    }

    cout << "Master closed access to shared memory, removed shared memory segment, and is exiting" << endl;
    return 0;
}
