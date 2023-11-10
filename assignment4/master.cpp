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

    sem_t* my_sem = sem_open(sem_name.c_str(), O_CREAT | O_EXCL, 0666, 1);
    if (my_sem == SEM_FAILED) {
        perror("sem_open failed");
        munmap(shared_data, sizeof(CLASS));
        close(shm_fd);
        return 1;
    }

    vector<pid_t> children;
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

    for (pid_t child : children) {
        int status;
        waitpid(child, &status, 0);
    }

    for (int i = 0; i < shared_data->index; ++i) {
        cout << "Child " << shared_data->response[i].child_number << " wrote lucky number " << shared_data->response[i].lucky_number << endl;
    }

    if (sem_close(my_sem) == -1) {
        perror("sem_close failed");
    }
    if (sem_unlink(sem_name.c_str()) == -1) {
        perror("sem_unlink failed");
    }

    if (munmap(shared_data, sizeof(CLASS)) == -1) {
        perror("munmap failed");
    }
    if (close(shm_fd) == -1) {
        perror("close failed");
    }
    if (shm_unlink(shm_name.c_str()) == -1) {
        perror("shm_unlink failed");
    }

    return 0;
}
