#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <openssl/ssl.h>
#include <openssl/err.h>

#define BUFFER_SIZE 4096 // Starting buffer size

int main(int argc, char *argv[])
{
  // Handle args
  char *host = argv[1];

  if (argc < 2)
  {
    printf("Usage: %s <hostname>\n", argv[0]);
    return 1;
  }

  printf("Hostname: %s\n", host);
  printf("Beginning Request\n");

  // Setup GET
  char http_get_request[512]; // Ensure the buffer is large enough
  snprintf(http_get_request, sizeof(http_get_request),
           "GET / HTTP/1.1\r\n"
           "Host: %s\r\n"
           "Connection: close\r\n"
           "User-Agent: C-HTTPS-Client\r\n"
           "\r\n",
           host);

  char *http_port = "80";
  char *https_port = "443";

  // Init OpenSSL
  SSL_library_init();
  SSL_CTX *ssl_ctx = SSL_CTX_new(TLS_client_method());
  if (!ssl_ctx)
  {
    // Handle error
    perror("SSL Context Failed");
    return 1;
  }

  // Get IP Address Info from Domain
  struct addrinfo hints, *res;
  memset(&hints, 0, sizeof(hints));
  hints.ai_family = AF_INET;       // IPv4
  hints.ai_socktype = SOCK_STREAM; // TCP

  if (getaddrinfo(host, https_port, &hints, &res) != 0)
  {
    // Handle error
    perror("Get Address Info Failed");
    return 1;
  }

  // Init Socket
  int sock = socket(res->ai_family, res->ai_socktype, res->ai_protocol);
  // Handle error
  if (sock == -1)
  {
    perror("Socket Creation Failed");
    return 1;
  }

  // Connect to the server
  if (connect(sock, res->ai_addr, res->ai_addrlen) == -1)
  {
    // Handle error
    perror("Socket Connection Failed");
    return 1;
  }

  // Establish SSL
  SSL *ssl = SSL_new(ssl_ctx);
  SSL_set_fd(ssl, sock);

  // Enable SNI (important for many servers)
  SSL_set_tlsext_host_name(ssl, host);

  // Ensure modern TLS options are used
  SSL_CTX_set_options(ssl_ctx, SSL_OP_ALL);

  if (SSL_connect(ssl) != 1)
  {
    ERR_print_errors_fp(stderr);  // Print detailed OpenSSL error
    perror("SSL Connection Failed");
    return 1;
  }

  // Send HTTP GET
  SSL_write(ssl, http_get_request, strlen(http_get_request));

  // Read
  char buffer[BUFFER_SIZE];
  while (1) {
    ssize_t bytesRead = SSL_read(ssl, buffer, BUFFER_SIZE - 1);
    if (bytesRead <= 0) break;  

    buffer[bytesRead] = '\0';  
    printf("%s", buffer);
}

  // Clean Up
  freeaddrinfo(res);

  SSL_shutdown(ssl);
  SSL_free(ssl);

  close(sock);

  SSL_CTX_free(ssl_ctx);

  return 0;
}