#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdint.h>

#include "err.h"

#define BUFFER_SIZE 5002

char shared_buffer[BUFFER_SIZE];

uint16_t read_port(char *string) {
    errno = 0;
    unsigned long port = strtoul(string, NULL, 10);
    PRINT_ERRNO();
    if (port > UINT16_MAX) {
        fatal("%ul is not a valid port number", port);
    }

    return (uint16_t) port;
}

int bind_socket(uint16_t port) {
    int socket_fd = socket(AF_INET, SOCK_DGRAM, 0); // creating IPv4 UDP socket
    ENSURE(socket_fd > 0);
    // after socket() call; we should close(sock) on any execution path;

    struct sockaddr_in server_address;
    server_address.sin_family = AF_INET; // IPv4
    server_address.sin_addr.s_addr = htonl(INADDR_ANY); // listening on all interfaces
    server_address.sin_port = htons(port);

    // bind the socket to a concrete address
    CHECK_ERRNO(bind(socket_fd, (struct sockaddr *) &server_address,
                     (socklen_t) sizeof(server_address)));

    return socket_fd;
}

size_t read_message(int socket_fd, struct sockaddr_in *client_address, char *buffer, size_t max_length) {
    socklen_t address_length = (socklen_t) sizeof(*client_address);
    int flags = 0; // we do not request anything special
    errno = 0;
    ssize_t len = recvfrom(socket_fd, buffer, max_length, flags,
                           (struct sockaddr *) client_address, &address_length);
    if (len < 0) {
        PRINT_ERRNO();
    }
    return (size_t) len;
}

void send_message(int socket_fd, const struct sockaddr_in *client_address, const char *message, size_t length) {
    socklen_t address_length = (socklen_t) sizeof(*client_address);
    int flags = 0;
    ssize_t sent_length = sendto(socket_fd, message, length, flags,
                                 (struct sockaddr *) client_address, address_length);
    ENSURE(sent_length == (ssize_t) length);
}

int main(int argc, char *argv[]) {
    if (argc != 2) {
        fatal("usage: %s <port>", argv[0]);
    }

    uint16_t port = read_port(argv[1]);
    printf("Listening on port %u\n", port);

    memset(shared_buffer, 0, sizeof(shared_buffer));

    int socket_fd = bind_socket(port);

//    FILE *fp = fopen("result.txt", "ab+");
//    if (fp == NULL) {
//        fatal("Could not open file");
//    }

    struct sockaddr_in client_address;
    size_t read_length;
    do {
        memset(shared_buffer, 0, sizeof(shared_buffer)); // clean the buffer
        read_length = read_message(socket_fd, &client_address, shared_buffer, sizeof(shared_buffer));

        FILE *fp = fopen("result.txt", "ab+");
        if (fp == NULL) {
            fatal("Could not open file");
        }

        if (strcmp(shared_buffer, "__exit__\n") == 0)
            break;
        fputs(shared_buffer, fp);
        fputs("\n\n", fp);


        printf("%zd\n", read_length); // note: we specify the length of the printed string
        //send_message(socket_fd, &client_address, shared_buffer, read_length);
        fclose(fp);

    } while (read_length > 0);

//    fclose(fp);
    printf("finished exchange\n");

    CHECK_ERRNO(close(socket_fd));

    return 0;
}
