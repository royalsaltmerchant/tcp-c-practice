#include <netinet/in.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

#include "http_parser.h"

#define BUFFER_SIZE 1024

typedef struct {
  int id;
  int socket;
} Client;

int on_url_cb(http_parser* parser, const char* at, size_t length) {
    // Access the client information through the parser's data field
  Client* client = (Client*)parser->data;

  // Handle the request URL
  printf("URL: %.*s\n", (int)length, at);

  if (length == 6 && strncmp(at, "/hello", 6) == 0 && parser->method == HTTP_GET) {
    // Load the HTML file
    FILE* htmlFile = fopen("index.html", "r");
    if (htmlFile == NULL) {
      perror("Failed to open HTML file");
      return 1;
    }

    // Send HTTP response headers
    char headers[BUFFER_SIZE];
    snprintf(headers, BUFFER_SIZE,
             "HTTP/1.1 200 OK\r\n"
             "Content-Type: text/html\r\n\r\n");
    send(client->socket, headers, strlen(headers), 0);

    // Send the HTML content
    char buffer[BUFFER_SIZE];
    size_t bytesRead;
    while ((bytesRead = fread(buffer, 1, BUFFER_SIZE, htmlFile)) > 0) {
      send(client->socket, buffer, bytesRead, 0);
    }

    fclose(htmlFile);
    close(parser->data);
  }

  else {
    // Send HTTP response headers
    char headers[BUFFER_SIZE];
    snprintf(headers, BUFFER_SIZE,
             "HTTP/1.1 500 Internal Server Error\r\n"
             "Content-Type: application/json\r\n\r\n");
    send(client->socket, headers, strlen(headers), 0);  }

  return 0;
}

void* handleClient(void* arg) {
  Client* new_client = (Client*)arg;

  // Initialize the HTTP parser
  http_parser parser;
  http_parser_init(&parser, HTTP_REQUEST);

  // Set the URL callback
  http_parser_settings settings;
  http_parser_settings_init(&settings);
  settings.on_url = on_url_cb;
  // Set the client information in the parser's data field
  parser.data = new_client;


  // Receive data from the server
  char buffer[BUFFER_SIZE];
  ssize_t bytesRead = recv(new_client->socket, buffer, BUFFER_SIZE - 1, 0);
  if (bytesRead == -1) {
    perror("Failed to receive data");
  } else if (bytesRead == 0) {
    // Connection closed by the server
    printf("Connection closed by the client\n");
  }

  // Process the received data with the HTTP parser
  http_parser_execute(&parser, &settings, buffer, bytesRead);

  close(new_client->socket);
  free(new_client);
  new_client = NULL;
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