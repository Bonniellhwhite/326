#include <iostream>
#include <string>
#include <vector>
#include <unistd.h>
#include <sys/wait.h>
using namespace std;

int main(int argc, char* argv[]) {
    //take list of names from arguments
    vector<std::string> names;             
    for (int i = 1; i < argc; i++) {
        names.push_back(argv[i]);
    }
    // print # of children
    int numChildren = names.size();
    cout << "I have " << numChildren << " children." << endl;

    // for every child in numChildren 
    for (int i = 0; i < numChildren; i++) {
        pid_t child_pid = fork();  // creating paralell process for child 
        if (child_pid == 0) { 
            string child_number = to_string(i + 1);
            execlp("./child", "./child", child_number.c_str(), names[i].c_str(), nullptr);
            }
    }

    int status;
    for (int i = 0; i < numChildren; i++) {
        wait(&status); // pauses parent for child to finish executing 
    }
    //done
    cout << "All child processes terminated. Parent exits." << endl;
    return 0;
}
