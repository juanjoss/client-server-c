#include "client.h"

int main(int argc, char const *argv[]) {
    // usage check
    if(argc != 3)
        err_die("usage: %s <server address>", argv[0]);

    int sockfd, n;
    int sendbytes;
    struct sockaddr_in servaddr;
    char sendLine[BUFF_SIZE];
    char recvLine[BUFF_SIZE];

    // creating a socket
    /*
        AF_INET = AF: Address Family, INET: Internet
        SOCK_STREAM = create connection -> send data -> receive data
        0: ID for TCP
    */
    if((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
        err_die("Error creating the socket.");

    bzero(&servaddr, sizeof servaddr);
    servaddr.sin_family = AF_INET;
    servaddr.sin_port = htons(SERVER_PORT); // htons = host to network, short

    // convert the ip to binary representation
    if(inet_pton(AF_INET, argv[1], &servaddr.sin_addr) <= 0)
        err_die("inet_pton error for: %s", argv[1]);

    if(connect(sockfd, (struct sockaddr *) &servaddr, sizeof servaddr) < 0)
        err_die("connection failed");

    // message
    sprintf(sendLine, "%s", argv[2]);
    sendbytes = strlen(sendLine);

    // send the request
    if(write(sockfd, sendLine, sendbytes) != sendbytes)
        err_die("write error");

    while ((n = read(sockfd, recvLine, BUFF_SIZE - 1) > 0)) {
        printf("%s", recvLine);
        memset(recvLine, 0, BUFF_SIZE);
    }
    if(n < 0)
        err_die("read error");

    return 0;
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