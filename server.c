#include "server.h"

int main(int argc, const char *argv[]) {
    int server_socket, client_socket;
    int addr_size = sizeof(struct sockaddr_in);
    struct sockaddr_in serv_addr, client_addr;

    // creating the threads
    for (int i = 1; i < THREAD_POOL_SIZE - 1; i++) {
        pthread_create(&thread_pool[i], NULL, thread_function, NULL);
    }
    
    check(server_socket = socket(AF_INET, SOCK_STREAM, 0), "socker error.");

    bzero(&serv_addr, sizeof serv_addr);
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = INADDR_ANY;
    serv_addr.sin_port = htons(SERVER_PORT); // htons = host to network, short

    check(bind(server_socket, (struct sockaddr *) &serv_addr, sizeof serv_addr), "bind error.");

    check(listen(server_socket, SERVER_BACKLOG), "listen error.");

    fd_set current_sockets, ready_sockets;
    int max_socket_size_so_far = 0;

    // initialize current set
    FD_ZERO(&current_sockets);
    FD_SET(server_socket, &current_sockets);

    max_socket_size_so_far = server_socket;

    while (true) {
        printf("waiting for connections on port: %d\n", SERVER_PORT);
        fflush(stdout);

        // because select() is destructive
        ready_sockets = current_sockets;

        if(select(max_socket_size_so_far, &ready_sockets, NULL, NULL, NULL) < 0) {
            perror("select error.");
            exit(EXIT_FAILURE);
        }

        for (int i = 0; i < max_socket_size_so_far; i++) {
            if(FD_ISSET(i, &ready_sockets)) {
                if(i == server_socket) {
                    // new connection
                    char client_address[BUFF_SIZE + 1];
                    
                    check(client_socket = accept(
                            server_socket, 
                            (struct sockaddr *) &client_addr, 
                            (socklen_t *) &addr_size
                        ), 
                        "accept error."
                    );

                    inet_ntop(AF_INET, &client_addr, client_address, BUFF_SIZE);
                    printf("\nClient connection: %s\n", client_address);

                    FD_SET(client_socket, &current_sockets);

                    if(client_socket > max_socket_size_so_far) {
                        max_socket_size_so_far = client_socket;
                    }
                }
                else {
                    /* single thread */
                        // handle_connection(client_socket);

                    /* multithread (no pool) */
                        // pthread_t t;
                        // int *pclient = malloc(sizeof(int));
                        // *pclient = client_socket;

                        // pthread_create(&t, NULL, handle_connection, pclient);

                    /* Multithread with fixed pool of threads */
                    int *pclient = malloc(sizeof(int));
                    *pclient = client_socket;

                    // critic zone
                    pthread_mutex_lock(&mutex);
                    
                    enqueue(pclient);
                    FD_CLR(i, &current_sockets);

                    pthread_cond_signal(&cond_var);
                    pthread_mutex_unlock(&mutex);
                }
            }
        }
    }

    return EXIT_SUCCESS;
}

void err_die(const char *fmt, ...) {
    int errno_save;
    va_list ap;

    errno_save = errno;

    // print fmt+args to stdout
    va_start(ap, fmt);
    vfprintf(stdout, fmt, ap);
    fprintf(stdout, "\n");
    fflush(stdout);

    // print error
    if(errno_save != 0) {
        fprintf(stdout, "(err: %d) %s\n", errno_save, strerror(errno_save));
        fprintf(stdout, "\n");
        fflush(stdout);
    }

    va_end(ap);

    exit(1);
}

void check(int exp, const char *msg) {
    if(exp == SOCKET_ERROR)
        err_die(msg);
}

char *bin2hex(const unsigned char *input, size_t len) {
    char *result;
    char *hex = "0123456789ABCDEF";

    if(input == NULL || len <= 0)
        return NULL;

    int resultLenth = (3 * len) + 1;

    result = malloc(resultLenth);
    bzero(result, resultLenth);

    for (int i = 0; i < len; i++) {
        result[3 * i] = hex[input[i] >> 4];
        result[(3 * i) + 1] = hex[input[i] & 0x0F];
        result[(3 * i) + 2] = ' ';
    }
    
    return result;
}

void *handle_connection(void *p_client_socket) {
    int client_socket = *((int *) p_client_socket);
    free(p_client_socket); // don't needed anymore

    char buffer[BUFF_SIZE];
    size_t bytes_read;
    int msg_size = 0;
    char actual_path[PATH_MAX + 1];
    
    while ((bytes_read = read(client_socket, buffer + msg_size, sizeof(buffer) - msg_size - 1)) > 0) {
        msg_size += bytes_read;

        if(msg_size > BUFF_SIZE - 1 || buffer[msg_size - 1] == '\n') break;
    }
    check(bytes_read, "recv error.");
    buffer[msg_size - 1] = 0;
    
    printf("REQUEST: %s\n", buffer);
    fflush(stdout);

    // validate path
    if(realpath(buffer, actual_path) == NULL) {
        printf("ERROR (bad path): %s\n", buffer);
        close(client_socket);
        return NULL;
    }

    // opening file
    FILE *f = fopen(actual_path, "r");
    if(f == NULL) {
        printf("ERROR (open): %s\n", buffer);
        close(client_socket);
        return NULL;
    }

    // read file and send data
    while((bytes_read = fread(buffer, 1, BUFF_SIZE, f)) > 0) {
        printf("sending %zu bytes\n", bytes_read);
        write(client_socket, buffer, bytes_read);
    }
    close(client_socket);
    fclose(f);
    printf("closing connection.\n");

    return NULL;
}

void* thread_function(void *arg) {
    while(true) {
        int *pclient;

        // critic zone
        pthread_mutex_lock(&mutex);
        
        if ((pclient = dequeue()) == NULL) {
            pthread_cond_wait(&cond_var, &mutex);

            // try again
            pclient = dequeue();
        }
        
        pthread_mutex_unlock(&mutex);

        // connection handling
        if(pclient != NULL) {
            handle_connection(pclient);
        }
    }
}
