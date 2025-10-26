#define NETWORK_IMPLEMENTATION
#include "../network.h"

int main() {
    struct sockaddr_storage client_storage;

    size_t buffer_size = 1024;
    char data[buffer_size];
    socket_t socket = network_listen("8080");
    socket_t newsocket =  network_accept(socket, &client_storage);
    while(1) {

        int bytes_recv = network_recv(newsocket, data, buffer_size);
        if (strcmp(data, ".exit\n") == 0) {
            break;
        }
        if(bytes_recv > 0) {
            printf("%s\n", data);
        }
        else if(bytes_recv == 0) {
            printf("Connection closed\n");
            break;
        }
        memset(data, '\0', buffer_size);
    }

    network_close(newsocket);
    network_close(socket);
}