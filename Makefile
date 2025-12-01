# Makefile for TCP Client/Server homework
CC = gcc
CFLAGS = -std=c11 -Wall -Wextra -O2 -pthread

# Source files
SERVER_SRC = tcp_server/server.c tcp_server/account.c
CLIENT_SRC = tcp_client/client.c 

# Output binaries (placed at root)
SERVER_BIN = server
CLIENT_BIN = client

# Default target: build both
all: $(SERVER_BIN) $(CLIENT_BIN)

# Build server
$(SERVER_BIN): $(SERVER_SRC)
	$(CC) $(CFLAGS) $(SERVER_SRC) -o $(SERVER_BIN)

# Build client
$(CLIENT_BIN): $(CLIENT_SRC)
	$(CC) $(CFLAGS) $(CLIENT_SRC) -o $(CLIENT_BIN)

# Clean
clean:
	rm -f $(SERVER_BIN) $(CLIENT_BIN) TCP_Server/*.o TCP_Client/*.o

.PHONY: all clean

