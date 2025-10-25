#define NETWORK_IMPLEMENTATION
#include "../headers/network.h"
#include <stdio.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <errno.h>
#include <string.h>

int main() {
    struct sockaddr_storage client_storage;

    size_t buffer_size = 1024;
    char data[buffer_size];
    socket_t socket = network_listen("8080");
    socket_t newsocket =  network_accept(socket, &client_storage);
    int bytes_recv = network_recv(newsocket, data, buffer_size);
    printf(data);
    network_close(newsocket);
    network_close(socket);
}