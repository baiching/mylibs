#define NETWORK_IMPLEMENTATION
#include <stdlib.h>

#include "../network.h"

int main() {
    struct addrinfo *clientdata, hint;
    int n = 256;
    char *buffer = malloc(n);

    memset(&hint, 0, sizeof(hint)); // removing garbage
    hint.ai_family = AF_UNSPEC;
    hint.ai_socktype = SOCK_STREAM;
    hint.ai_flags = AI_PASSIVE;

    getaddrinfo(NULL, "8080", &hint, &clientdata);
    socket_t sockfd = socket(clientdata->ai_family, clientdata->ai_socktype, clientdata->ai_protocol);

    sockfd = network_connect(clientdata);
    while(1) {
        fgets(buffer, sizeof(buffer), stdin);
        int bytes = network_send(sockfd, buffer);
        if (strcmp(buffer, ".exit\n") == 0) {
            break;
        }
    }
    free(buffer);
    network_close(sockfd);
}