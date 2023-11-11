#include <iostream>
#include <string>
#include <fcntl.h>
#include <unistd.h>
#include <sys/mman.h>
#include <semaphore.h>
#include "myShm.h" // Ensure this is the correct header file name

using namespace std;

int main(int argc, char* argv[]) {

    cout << "Slave begins execution" << endl;    
    if (argc != 4) {
        cerr << "Usage: " << argv[0] << " <child_number> <shm_name> <sem_name>" << endl;
        return 1;
    }

   

    int child_number = stoi(argv[1]);
    string shm_name = argv[2];
    string sem_name = argv[3];

    cout << "I am child number " << child_number << ", received shared memory name "<< shm_name << " and " << sem_name << endl;

    int shm_fd = shm_open(shm_name.c_str(), O_RDWR, 0666);
    if (shm_fd == -1) {
        perror("shm_open failed");
        return 1;
    }

    CLASS* shared_data = static_cast<CLASS*>(mmap(NULL, sizeof(CLASS), PROT_READ | PROT_WRITE, MAP_SHARED, shm_fd, 0));
    if (shared_data == MAP_FAILED) {
        perror("mmap failed");
        close(shm_fd);
        return 1;
    }

    sem_t* my_sem = sem_open(sem_name.c_str(), 0);
    if (my_sem == SEM_FAILED) {
        perror("sem_open failed");
        munmap(shared_data, sizeof(CLASS));
        close(shm_fd);
        return 1;
    }

    if (sem_wait(my_sem) == -1) {
        perror("sem_wait failed");
        sem_close(my_sem);
        munmap(shared_data, sizeof(CLASS));
        close(shm_fd);
        return 1;
    }

    // using two slots
    int idx = shared_data->index*2;

    // childing number in 1st slot 
    shared_data->response[idx] = child_number;

    cout << "Child number " << child_number << ": What is my lucky number?" << endl;
    cin >> shared_data->response[idx+1];

    // increment by 2 
    shared_data->index++;

    cout << "I have written my child number to slot " << idx <<" and my lucky number to slot " << idx+1 << ", and updated index to" << idx+2 << endl;

    if (sem_post(my_sem) == -1) {
        perror("sem_post failed");
    }

    if (sem_close(my_sem) == -1) {
        perror("sem_close failed");
    }
    if (munmap(shared_data, sizeof(CLASS)) == -1) {
        perror("munmap failed");
    }
    if (close(shm_fd) == -1) {
        perror("close failed");
    }

    cout << "Child x closed access to shared memory and terminates. " << endl;
    return 0;
}
