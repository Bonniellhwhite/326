#include <cstring>
#include <iostream>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <unistd.h>

using namespace std;

struct MyMessage {
    long messageType;
    char messageText[1024]; 
};

int main(int argc, char* argv[]) {
    int processId = getpid();
    int messageId = stoi(argv[1]);
    long messageType = stol(argv[2]);

    // Print Information
    cout << "Receiver, PID " << processId << ", begins execution\nReceiver with PID " << processId 
        << " received message queue id " << messageId << " through command line argument\n";
    sleep(3);

    struct MyMessage receivedMessage;
    while (true) {
        // Receive message from the message queue
        ssize_t received = msgrcv(messageId, &receivedMessage, sizeof(receivedMessage.messageText), messageType, 0);

        if (received < 0) {
            if (errno == ENOMSG) {
                // No messages in the queue, keep waiting
                continue;
            } else {
                perror("msgrcv");
                return 1;
            }
        } else {
            // Message received successfully
            cout << "Receiver with PID " << processId << " retrieved the following message from this message queue:" 
                << receivedMessage.messageText << endl;
            break;
        }
    }

    sleep(3);
    struct MyMessage acknowledgmentMessage;
    acknowledgmentMessage.messageType = messageType;
    strcpy(acknowledgmentMessage.messageText, (string("Receiver with PID ") + to_string(processId) + 
        string(" acknowledges receipt of message")).c_str());
    
    // Send acknowledgment message into the message queue
    if (msgsnd(messageId, &acknowledgmentMessage, sizeof(acknowledgmentMessage.messageText), 0) >= 0) {
        cout << "Receiver with PID " << processId << " sent the following acknowledgement message into the message queue "
            << "Receiver with PID " << processId << " acknowledges receipt of message\nReceiver with PID " << processId << 
            " terminates\n";
    }
    return 0;
}
