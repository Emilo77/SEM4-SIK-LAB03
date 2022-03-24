#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdint.h>
#include <arpa/inet.h>

#include "err.h"

#define BUFFER_SIZE 20

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

struct sockaddr_in get_send_address(char *host, uint16_t port) {
    struct addrinfo hints;
    memset(&hints, 0, sizeof(struct addrinfo));
    hints.ai_family = AF_INET; // IPv4
    hints.ai_socktype = SOCK_DGRAM;
    hints.ai_protocol = IPPROTO_UDP;

    struct addrinfo *address_result;
    CHECK(getaddrinfo(host, NULL, &hints, &address_result));

    struct sockaddr_in send_address;
    send_address.sin_family = AF_INET; // IPv4
    send_address.sin_addr.s_addr =
            ((struct sockaddr_in *) (address_result->ai_addr))->sin_addr.s_addr; // IP address
    send_address.sin_port = htons(port); // port from the command line

    freeaddrinfo(address_result);

    return send_address;
}

void send_message(int socket_fd, const struct sockaddr_in *send_address, const char *message) {
    size_t message_length = strnlen(message, BUFFER_SIZE);
    if (message_length == BUFFER_SIZE) {
        fatal("parameters must be less than %d characters long", BUFFER_SIZE);
    }
    int send_flags = 0;
    socklen_t address_length = (socklen_t) sizeof(*send_address);
    errno = 0;
    ssize_t sent_length = sendto(socket_fd, message, message_length, send_flags,
                                 (struct sockaddr *) send_address, address_length);
    if (sent_length < 0) {
        PRINT_ERRNO();
    }
    ENSURE(sent_length == (ssize_t) message_length);
}

size_t receive_message(int socket_fd, struct sockaddr_in *receive_address, char *buffer, size_t max_length) {
    int receive_flags = 0;
    socklen_t address_length = (socklen_t) sizeof(*receive_address);
    errno = 0;
    ssize_t received_length = recvfrom(socket_fd, buffer, max_length, receive_flags,
                                       (struct sockaddr *) receive_address, &address_length);
    if (received_length < 0) {
        PRINT_ERRNO();
    }
    return (size_t) received_length;
}


int main(int argc, char *argv[]) {
    if (argc < 5) {
        fatal("Usage: %s <host> <port> <message> <package number> <data size>...\n", argv[0]);
    }

    char *host = argv[1];
    uint16_t port = read_port(argv[2]);
    struct sockaddr_in send_address = get_send_address(host, port);

    int socket_fd = socket(PF_INET, SOCK_DGRAM, 0);
    if (socket_fd < 0) {
        PRINT_ERRNO();
    }

    char *client_ip = inet_ntoa(send_address.sin_addr);
    uint16_t client_port = ntohs(send_address.sin_port);

    for (int i = 5; i < argc; i++) {
        send_message(socket_fd, &send_address, argv[i]);
        printf("sent to %s:%u: '%s'\n", client_ip, client_port, argv[i]);
        memset(shared_buffer, 0, sizeof(shared_buffer)); // clean the buffer
        size_t max_length = sizeof(shared_buffer) - 1; // leave space for the null terminator
        size_t received_length = receive_message(socket_fd, &send_address, shared_buffer, max_length);
        printf("received %zd bytes from %s:%u: '%s'\n", received_length, client_ip, client_port, shared_buffer);
    }

    CHECK_ERRNO(close(socket_fd));

    return 0;
}
