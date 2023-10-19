#include <iostream>
#include <string>
using namespace std;
int main(int argc, char* argv[]) {
    if (argc = 2) { // there must be 2 arguments
        string childNumber = argv[1]; // extracting child # from arguments
        string childName = argv[2]; // extracting child name from arguments
        cout << "I am child number " << childNumber << ", and my name is " << childName << "." << endl;
    }
    return 0;
}


