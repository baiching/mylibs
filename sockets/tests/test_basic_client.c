#define NETWORK_IMPLEMENTATION
#include "../network.h"

int main() {
    struct addrinfo *clientdata, hint;

    memset(&hint, 0, sizeof(hint));
    hint.ai_family = AF_UNSPEC;
    hint.ai_socktype = SOCK_STREAM;
    hint.ai_flags = AI_PASSIVE;

    getaddrinfo(NULL, "8080", &hint, &clientdata);
    socket_t sockfd = socket(clientdata->ai_family, clientdata->ai_socktype, clientdata->ai_protocol);
    sockfd = network_connect(clientdata);
    int bytes = network_send(sockfd, "hello");
    printf("%d bytes sent\n", bytes);
}