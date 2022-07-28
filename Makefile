CC = gcc
CFLAGS = -g -Wall
CLIENT_OBJS = client.o 
SERVER_OBJS = server.o queue.o
CLIENT = client
SERVER = server
SUBMIT = submit
SUBMIT_NAME = client_server.zip

all: $(CLIENT) $(SERVER)

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

client: $(CLIENT_OBJS)
	$(CC) $(CFLAGS) $(CLIENT_OBJS) -o client

server:	$(SERVER_OBJS)
	$(CC) $(CFLAGS) -pthread $(SERVER_OBJS) -o server

clean:
	rm client.o server.o client server client_server.zip

submit:
	$(RM) $(SUBMIT_NAME)
	zip $(SUBMIT_NAME) $(CLIENT) $(SERVER)
