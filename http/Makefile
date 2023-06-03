CC = gcc
CFLAGS = -Wall -Wextra -pedantic
LDFLAGS = -lhttp_parser

httpserver: httpserver.c
	$(CC) $(CFLAGS) -o httpserver httpserver.c $(LDFLAGS)

clean:
	rm -f httpserver
