#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>
#include <string.h>

#include <netinet/in.h>

int main()
{

  // open a file to serve
  FILE *html_data;
  html_data = fopen("index.html", "r");

  char response_data[1024];
  fgets(response_data, 1024, html_data);

  char http_header[2048] = "HTTP/1.1 200 OK\r\n\n";
  strcat(http_header, response_data);

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

  // loop for continuous behavior
  int client_socket;
  while (1)
  {
    client_socket = accept(server_socket, NULL, NULL);
    send(client_socket, http_header, sizeof(http_header), 0);
    close(client_socket);
  }

  return 0;
}