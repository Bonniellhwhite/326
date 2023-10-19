#include <cstring>
#include <iostream>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <unistd.h>

using namespace std;

struct MyMessage {
    long messageType;
    char messageText[1024]; // Adjust the buffer size as needed
};

int main(int argc, char* argv[]) {
    int processId = getpid();
    int messageId = stoi(argv[1]);
    string message;
    long messageType = stol(argv[2]);

    // Display information about the sender process
    cout << "Sender, PID " << processId << ", begins execution" << endl
         << "Sender with PID " << processId << " received message queue id " << messageId <<
            " through command line argument" << endl
         << "Sender with PID " << processId << ": Please input your message" << endl;
    cin >> message;
    cout << "Sender with PID " << processId << " sent the following message into the queue: " << message << "\n";
    sleep(3);

    // Package the message for sending
    struct MyMessage messagePackage;
    messagePackage.messageType = messageType;
    strcpy(messagePackage.messageText, message.c_str());

    // Send the message to the message queue
    msgsnd(messageId, &messagePackage, sizeof(messagePackage.messageText), MSG_NOERROR);

    // Receive acknowledgment message
    struct MyMessage receivedMessage;
    while (true) {
        ssize_t received = msgrcv(messageId, &receivedMessage, sizeof(messagePackage.messageText), messageType, 0);

        if (received < 0) {
            if (errno == ENOMSG) {
                // No messages in the queue, so continue waiting.
                // You can add a sleep or other waiting mechanism here if needed.
                continue;
            } else {
                perror("msgrcv");
                return 1;
            }
        } else {
            // Message received successfully
            cout << "Sender with PID " << processId << " retrieved the following acknowledgement message: " <<
                receivedMessage.messageText << endl;
            break;
        }
        cout << "Sender with PID " << processId << " terminates\n";
    }
    return 0;
}
