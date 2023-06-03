#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>
#include <netinet/in.h>
#include <fcntl.h>
#include <errno.h>

#define BUFFER_SIZE 1024

const char CLIENT_INIT_MESSAGE[256] = "Hello I am here...";

int main() {
    // Create a socket
    int network_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (network_socket == -1) {
        perror("Failed to create socket");
        exit(1);
    }

    // Set the socket to non-blocking mode
    int flags = fcntl(network_socket, F_GETFL, 0);
    fcntl(network_socket, F_SETFL, flags | O_NONBLOCK);

    // Specify an address for the socket
    struct sockaddr_in server_address;
    server_address.sin_family = AF_INET;
    server_address.sin_port = htons(8000);
    server_address.sin_addr.s_addr = INADDR_ANY;

    int connection_status = connect(network_socket, (struct sockaddr*)&server_address, sizeof(server_address));
    if (connection_status == -1 && errno != EINPROGRESS) {
        perror("There was an error making a connection to the server");
        exit(1);
    }

    send(network_socket, CLIENT_INIT_MESSAGE, sizeof(CLIENT_INIT_MESSAGE), 0);

    // Set up the file descriptors for select
    fd_set readSet;
    int stdinFd = fileno(stdin);
    int maxFd = (network_socket > stdinFd) ? network_socket : stdinFd;

    while (1) {
        FD_ZERO(&readSet);
        FD_SET(network_socket, &readSet);
        FD_SET(stdinFd, &readSet);

        // Use select to wait for activity on either the network socket or stdin
        int activity = select(maxFd + 1, &readSet, NULL, NULL, NULL);
        if (activity == -1) {
            perror("Select error");
            break;
        }

        // Data available from the server
        if (FD_ISSET(network_socket, &readSet)) {
            // Receive data from the server
            char read_buffer[BUFFER_SIZE];
            memset(read_buffer, 0, BUFFER_SIZE); // Clear the read buffer
            ssize_t bytesRead = recv(network_socket, read_buffer, BUFFER_SIZE - 1, 0);
            if (bytesRead == -1) {
                perror("Failed to receive data");
                break;
            } else if (bytesRead == 0) {
                // Connection closed by the server
                printf("Connection closed by the server\n");
                break;
            }

            // Process and display the received data
            printf("%s\n", read_buffer);
        }

        // Data available from stdin (user input)
        if (FD_ISSET(stdinFd, &readSet)) {
            // Read input from stdin using scanf
            char write_buffer[BUFFER_SIZE];
            memset(write_buffer, 0, BUFFER_SIZE); // Clear the write buffer
            if (scanf("%s", write_buffer) == 1) {
                // Send the input message to the server
                ssize_t bytesSent = send(network_socket, write_buffer, strlen(write_buffer), 0);
                if (bytesSent == -1) {
                    perror("Failed to send message to server");
                    break;
                }
            } else {
                break; // Exit the loop on scanf error or EOF
            }
        }
    }

    close(network_socket);
    return 0;
}

  // close socket
  close(network_socket);
  return 0;
}