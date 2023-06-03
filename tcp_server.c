#include <netinet/in.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

#define BUFFER_SIZE 1024

const char SERVER_INIT_MESSAGE[256] = "You have connected to the SALT server";

typedef struct {
  int id;
  int socket;
} Client;

void* handleClient(void* arg) {
  Client* new_client = (Client*)arg;
  send(new_client->socket, SERVER_INIT_MESSAGE, sizeof(SERVER_INIT_MESSAGE), 0);

  // Receive and process multiple responses
  char buffer[BUFFER_SIZE];
  while (1) {
    memset(buffer, 0, BUFFER_SIZE);  // Clear the buffer

    // Receive data from the server
    ssize_t bytesRead = recv(new_client->socket, &buffer, BUFFER_SIZE - 1, 0);
    if (bytesRead == -1) {
      perror("Failed to receive data");
      break;
    } else if (bytesRead == 0) {
      // Connection closed by the server
      printf("Connection closed by the client\n");
      break;
    }
    // Process and display the received data
    printf("Received data: %s\n", buffer);
    send(new_client->socket, buffer, sizeof(buffer), 0);
  }

  close(new_client->socket);
  pthread_exit(NULL);
}

int main() {
  // create the server socket
  int server_socket;
  server_socket = socket(AF_INET, SOCK_STREAM, 0);

  // define server address
  struct sockaddr_in server_address;
  server_address.sin_family = AF_INET;
  server_address.sin_port = htons(8000);
  server_address.sin_addr.s_addr = INADDR_ANY;

  // bind the socket to the IP port
  bind(server_socket, (struct sockaddr*)&server_address, sizeof(server_address));
  // listen
  listen(server_socket, 3);

  while (1) {
    // accept client sockets
    int client_socket;
    client_socket = accept(server_socket, NULL, NULL);
    if (client_socket == -1) {
      perror("Failed to accept client connection");
      exit(1);
    }
    // send message to a client

    Client* new_client = malloc(sizeof(Client));
    new_client->id = 1;
    new_client->socket = client_socket;

    pthread_t threadId;
    int result = pthread_create(&threadId, NULL, handleClient, (void*)new_client);

    if (result != 0) {
      perror("Failed to spawn thread");
      exit(1);
    }
    pthread_detach(threadId);
  }

  // close socket
  // close(server_socket);

  return 0;
}