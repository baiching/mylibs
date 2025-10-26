#define NETWORK_IMPLEMENTATION
#include <stdlib.h>

#include "../network.h"

int main() {
    struct addrinfo *clientdata = NULL;
    int n = 1024;
    char *buffer = malloc(n);

    socket_t sockfd = network_connect(clientdata);
    while(1) {
        fgets(buffer, sizeof(buffer), stdin);
        int bytes = network_send(sockfd, buffer);
        if (strcmp(buffer, ".exit\n") == 0) {
            break;
        }

        memset(buffer, '\0', n);
    }
    free(buffer);
    network_close(sockfd);
}