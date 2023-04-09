#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>

#include <netinet/in.h>

int main()
{

  char server_message[256] = "You have connected to the SALT server";

  // create the server socket
  int server_socket;
  server_socket = socket(AF_INET, SOCK_STREAM, 0);

  // define server address
  struct sockaddr_in server_address;
  server_address.sin_family = AF_INET;
  server_address.sin_port = htons(8000);
  server_address.sin_addr.s_addr = INADDR_ANY;

  // bind the socket to the IP port
  bind(server_socket, (struct sockaddr *)&server_address, sizeof(server_address));
  // listen
  listen(server_socket, 3);
  // accept client sockets
  int client_socket;
  client_socket = accept(server_socket, NULL, NULL);
  // send message to a client
  send(client_socket, server_message, sizeof(server_message), 0);

  // close socket
  close(server_socket);

  return 0;
}