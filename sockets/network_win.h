/**
* @file network.h
 * @brief Minimal Linux Socket Library - Lightweight TCP networking for Windows
 * @version 0.1
 *
 * Header-only socket library providing clean TCP networking APIs for Linux.
 *
 * Available APIs:
 * Server:
 *   • network_listen(const char *port) - Listen on port (all interfaces). the value of parameter port is a string
 *   • network_listen_on()    - Listen on specific IP and port
 *   • network_accept()       - Accept incoming connections
 *
 * Client:
 *   • network_connect()      - Connect to server using addrinfo
 *
 * Data Transfer:
 *   • network_send()         - Send null-terminated string
 *   • network_recv()         - Receive data into buffer
 *   • network_close()        - Close socket connection
 *
 * Concurrency:
 *
 *
 */
#ifndef NETWORK_WIN_H
#define NETWORK_WIN_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>


#ifdef __cplusplus
extern "C" {
#endif

#ifdef _WIN32_WINNT
#undef _WIN32_WINNT
#endif
#define _WIN32_WINNT 0x0600

#define WIN32_LEAN_AND_MEAN
#include <winsock2.h>
#include <ws2tcpip.h>
#include <windows.h>

#pragma comment(lib, "ws2_32.lib")

typedef SOCKET socket_t;


#define BACKLOG 10 //only accepts upto 10 connections

// To store buffer better, this way makes the buffer more flexible
typedef struct {
    char *buffer;
    size_t size;
} data;

// return codes
typedef enum {
    NETWORK_SUCCESS = 0,

    // Generic Errors
    SOCKET_CREATE_FAILED,
    SOCKET_BIND_FAILED,
    SOCKET_LISTEN_FAILED,
    SOCKET_ACCEPT_FAILED,
    SOCKET_CONNECT_FAILED,
    SOCKET_INVALID,
    SOCKET_UNKNOWN_ERROR,
} network_result;

// windows only cycle
int network_init(void);

/**
 * This functioin will remove all the sockets
 */
void network_cleanup(void);

// server side
socket_t network_listen(const char *port);
socket_t network_listen_on(const char *ip, const char *port); // specific interface
socket_t network_accept(socket_t socktfd, struct sockaddr_storage *client_storage); // accept connection

// client side
socket_t network_connect(struct addrinfo *server_address);
network_result network_connect_timeout(const char *ip, int port, int timeout_ms);

// data transfer
socket_t network_send(socket_t socket, const void *data);
socket_t network_recv(socket_t socketfd, void *data, size_t buffer_size);

// Guaranteed Delivery
socket_t network_send_all(socket_t sockfd, void *data, size_t buffer_size); // TODO

// closing socket
void network_close(socket_t socket);

// Utilities
static void network_win_errmsg(DWORD errcode);

void network_set_nonblocking(socket_t sock); // sets the file descriptor of the socket as non-blocking, returns nothing
void network_would_block(socket_t sock); // sets the file descriptor of the socket as blocking, returns nothing


#ifdef NETWORK_WIN_IMPLEMENTATION

static void network_win_errmsg(DWORD errcode) {

    // Buffer to store the error message
    LPWSTR errormsg = NULL;

    FormatMessageW(
            FORMAT_MESSAGE_ALLOCATE_BUFFER |
            FORMAT_MESSAGE_FROM_SYSTEM |
            FORMAT_MESSAGE_IGNORE_INSERTS,
            NULL,
            errcode,
            MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
            (LPWSTR)&errormsg,
            0,
            NULL
        );

    wprintf(L"%lu: %s\n", errcode, errormsg);
    if (errormsg) LocalFree(errormsg);

}

inline int network_init(void) {
    WORD wVersionRequested;
    WSADATA wsaData;
    int err;

    /* Use the MAKEWORD(lowbyte, highbyte) macro declared in Windef.h */
    wVersionRequested = MAKEWORD(2, 2);
    err = WSAStartup(wVersionRequested, &wsaData);

    if (err != 0 ) {
        printf("WSAStartup failed with error: %d\n", err);
        return 1;
    }
    return 0;
}

inline void network_cleanup(void) {
    if (WSACleanup() != 0) {
        printf("WSACleanup failed with error: %d\n", WSAGetLastError());
    }
    printf("cleanup successful!\n");
}

inline socket_t network_listen(const char *port) {
    struct addrinfo hints, *res;
    socket_t sockfd;

    // clearnig the memory to load by getaddrinfo
    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_INET; // ipv4
    hints.ai_socktype = SOCK_STREAM; // Since TCP, we'll be using streaming to transfer the data
    hints.ai_flags = AI_PASSIVE; // fill up any available ip

    getaddrinfo(NULL, port, &hints, &res);

    // bind the socket to the ip and port
    sockfd = socket(res->ai_family, res->ai_socktype, res->ai_protocol);
    // check if socket is created
    if(sockfd < 0) {
        printf("Socket creation failed at Listen API.\n");
        freeaddrinfo(res);
        return -1;
    }
    if(bind(sockfd, res->ai_addr, res->ai_addrlen) < 0) {
        printf("Bind has failed!\n");
        freeaddrinfo(res);
        network_close(sockfd);
        return -1;
    }

    if(listen(sockfd, BACKLOG) < 0) {
        printf("Listen has failed!\n");
        freeaddrinfo(res);
        network_close(sockfd);
        return -1;
    }

    freeaddrinfo(res);

    return sockfd;
}

inline socket_t network_listen_on(const char *ip, const char *port) {
    struct addrinfo hints, *res;
    socket_t sockfd;

    // clearing the memory to load by getaddrinfo
    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_INET; // ipv4
    hints.ai_socktype = SOCK_STREAM; // Since TCP, we'll be using streaming to transfer the data
    hints.ai_flags = AI_PASSIVE; // fill up any available ip

    getaddrinfo(ip, port, &hints, &res);

    // bind the socket to the ip and port
    sockfd = socket(res->ai_family, res->ai_socktype, res->ai_protocol);
    // check if socket is created
     if(sockfd < 0) {
        printf("Socket creation failed at Listen API.\n");
        freeaddrinfo(res);
        return -1;
    }
    if(bind(sockfd, res->ai_addr, res->ai_addrlen) < 0) {
        printf("Bind has failed!\n");
        freeaddrinfo(res);
        network_close(sockfd);
        return -1;
    }

    if(listen(sockfd, BACKLOG) < 0) {
        printf("Listen has failed!\n");
        freeaddrinfo(res);
        network_close(sockfd);
        return -1;
    }

    freeaddrinfo(res);

    return sockfd;
}

inline socket_t network_accept(socket_t socktfd, struct sockaddr_storage *client_storage) {
    if (socktfd < 0) {
        printf("socket file descriptor is NULL.\n");
        return -1;
    }
    if (client_storage == NULL) {
        printf("Address is NULL.\n");
        return -1;
    }
    socklen_t address_len = sizeof(*client_storage);
    socket_t newfd = accept(socktfd, (struct sockaddr *) client_storage, &address_len);

    if (newfd < 0) {
        printf("Accept has failed.\n");
        return -1;
    }
    printf("Client connected.\n");
    return newfd;
}

inline socket_t network_connect(struct addrinfo *server_address) {
    // if user sends and empty addrinfo, load the data here
    if (server_address == NULL || server_address->ai_family == 0) {
        struct addrinfo hint;
        memset(&hint, 0, sizeof(hint)); // removing garbage
        hint.ai_family = AF_UNSPEC;
        hint.ai_socktype = SOCK_STREAM;
        hint.ai_flags = AI_PASSIVE;

        getaddrinfo(NULL, "8080", &hint, &server_address);
    }

    socket_t client_socket = socket(server_address->ai_family,
        server_address->ai_socktype,
        server_address->ai_protocol);

    if (client_socket < 0) {
        printf("Socket creation failed. %s\n", strerror(errno));
        return -1;
    }

    if (connect(client_socket, server_address->ai_addr, server_address->ai_addrlen) == 0) {
        printf("Socket successfully connected.\n");
        freeaddrinfo(server_address);
        return client_socket;
    }
    else {
        network_win_errmsg(GetLastError()); // prints the exact error

        freeaddrinfo(server_address);
        network_close(client_socket);
        return -1;
    }
}

inline socket_t network_send(socket_t socketfd, const void *data) {
    if (socketfd < 0) {
        printf("Invalid socket descriptor: %d\n", socketfd);
        exit(EXIT_FAILURE);
        //return -1;
    }

    if (data == NULL) {
        printf("Data pointer is NULL\n");
        return -1;
    }

    int len = strlen(data);
    if (len == 0) {
        printf("Attempting to send empty string\n");
        return 0;
    }

    int bytes_sent = send(socketfd, data, len, 0);
    if(bytes_sent < 0) {
        printf("send failed.\n");
        return -1;
    }
    return bytes_sent;
}

inline socket_t network_recv(socket_t socketfd, void *data, size_t buffer_size){
    if (socketfd < 0) {
        printf("Invalid socket descriptor: %d\n", socketfd);
        //return -1;
        exit(EXIT_FAILURE);
    }

    if (data == NULL) {
        printf("Buffer pointer is NULL, no place to put the data\n");
        return -1;
    }

    if (buffer_size == 0) {
        printf("Buffer size is zero, no space for data insertion.\n");
        return -1;
    }
    int bytes_recv = recv(socketfd, data, buffer_size, 0);
    if(bytes_recv < 0) {
        network_win_errmsg(GetLastError()); // prints the exact error
        return -1;
    }
    return bytes_recv;

}

inline socket_t network_send_all(socket_t sockfd, void *data, size_t buffer_size) {
    printf("don't use it, it's haven't been implemented yet!\n");
    exit(EXIT_FAILURE);
    return 1;
}
// Concurrency
inline void network_set_nonblocking(socket_t sock) {
    unsigned long nonblocking = 1;
    if (ioctlsocket(sock, FIONBIO, &nonblocking) < 0) {
        printf("Non-blocking failed.\n");
        exit(EXIT_FAILURE);
    }
    return;

}

inline void network_would_block(socket_t sock) {
    printf("Windows doesn't support epoll unfortunately!.\n");
    return;
}

inline void network_close(socket_t socket) {
    closesocket(socket);
}

static inline FILE* network_load_file(const char *filename) {
    return fopen(filename, "rb");
}
#endif

#ifdef __cplusplus
}
#endif

#endif
