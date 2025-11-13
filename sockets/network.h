/**
 * @file network.h
 * @brief Minimal Linux Socket Library - Lightweight TCP networking for Linux
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

#ifndef NETWORK_H
#define NETWORK_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

#if defined(_WIN32) || defined(__MINGW32__)
#define WINSOCK_IMPL
#elif defined(__linux__)
#define LINUX_SOCKETS_IMPL
#elif defined(__APPLE__)
#error "macOS is not supported yet."
#endif


#ifdef __cplusplus
extern "C" {
#endif

#ifdef WINSOCK_IMPL
#include <winsock2.h>
#include <ws2tcpip.h>
#include <mswsock.h>

typedef SOCKET socket_t;
#elif defined(LINUX_SOCKETS_IMPL)
#include <sys/socket.h>
#include <netinet/in.h>
#include <errno.h>
#include <unistd.h>
#include <netdb.h>
#include <fcntl.h>
#include <sys/epoll.h>

typedef int socket_t;
#endif

#define BACKLOG 10 //only accepts upto 10 connections

// To store buffer better, this way makes the buffer more flexible
typedef struct {
    char *buffer;
    size_t size;
} data;

// it'll be used for epoll event to store users data
struct client_event_data {
    int efd; // epoll_fd: fd of the current epoll
    int op; // type of operation needed to perform: EPOLL_CTL_ADD, EPOLL_CTL_MOD, EPOLL_CTL_DEL
    int clientfd; // the socket this event will be monitoring
    uint32_t event; // the type of event that should be looked out for
};

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
void network_set_nonblocking(socket_t sock); // sets the file descriptor of the socket as non-blocking, returns nothing
void network_would_block(socket_t sock); // sets the file descriptor of the socket as blocking, returns nothing

// EPOLL events
socket_t network_epoll_create(void); // creates a new epoll and returns epoll_fd which is an integer
void network_epoll_ctl(struct client_event_data *cdata); // adds the clients fd in the event to monitor
/**
 *
 * @param epollfd : The efd of the current event
 * @param events : Example below on how to allocate data for stack or heap
 * @param maxevents : Maximum number of events return, must be greater than zero
 * @param timeout: The wait will block for specific timeout. Specifies the number of milliseconds.
 * @warning Specifying a timeout of -1 causes epoll_wait() to block indefinitely
 * @return Number of fd's that are ready for I/O operations
 *
 * @example
 * // For stack
 * struct epoll_event events[maxevents];
 *
 * // for heap
 * struct epoll_event *events = malloc(maxevents * sizeof(struct epoll_event));
 */
int network_epoll_wait(socket_t epollfd, struct epoll_event *events, int maxevents, int timeout); // returns the number of fd's are ready for I/O operations
void network_epoll_close(socket_t epollfd); // Should close the event gracefully, after closing all the sockets it's managing

#ifdef NETWORK_IMPLEMENTATION

inline int network_init(void) {
#ifdef WINSOCK_IMPL
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
#elif LINUX_SOCKETS_IMPL
    printf("This function is for Windows only, it's not needed in linux.\n");
#endif


    return 0;
}

inline void network_cleanup(void) {
#ifdef WINSOCK_IMPL
    if (WSACleanup() != 0) {
        printf("WSACleanup failed with error: %d\n", WSAGetLastError());
    }
#elif LINUX_SOCKETS_IMPL
    printf("Please use network_close(socket_fd), this function is windows only.\n");
#endif

}

inline socket_t network_listen(const char *port) {
    struct addrinfo hints, *res;
    socket_t sockfd;

    // clearnig the memory to load by getaddrinfo
    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_UNSPEC; // ipv4 or ipv6
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

    // clearnig the memory to load by getaddrinfo
    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_UNSPEC; // ipv4 or ipv6
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
        printf("Connection failed. %s\n", strerror(errno));
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
        printf("recv failed. %s\n", strerror(errno));
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
    int originslflags = fcntl(sock, F_GETFL, 0);
    if (fcntl(sock, F_SETFL, originslflags | O_NONBLOCK) < 0) {
        printf("fcntl F_SETFL O_NONBLOCK failed.\n");
        exit(EXIT_FAILURE);
    }

}

inline void network_would_block(socket_t sock) {
    int originslflags = fcntl(sock, F_GETFL, 0);
    if (fcntl(sock, F_SETFL, originslflags & O_NONBLOCK) < 0) {
        printf("fcntl F_SETFL O_NONBLOCK failed.\n");
        exit(EXIT_FAILURE);
    }
}

/* Epoll events */
inline socket_t network_epoll_create(void) {
    int epollfd = epoll_create(1);
    if (epollfd < 0) {
        printf("epoll_create failed.\n");
        exit(EXIT_FAILURE);
    }
    return epollfd;
}

inline void network_epoll_ctl(struct client_event_data *cdata) {
    struct epoll_event ev;

    int epollfd = cdata->efd;
    int op = cdata->op;
    int fd = cdata->clientfd;

    ev.events = cdata->event;
    ev.data.fd = cdata->clientfd;

    if (cdata->op == EPOLL_CTL_DEL) {
        epoll_ctl(epollfd, op, fd, NULL);
        printf("DELETED EVENT\n");
        return;
    }

    if (epoll_ctl(epollfd, op, fd, &ev) < 0) {
        printf("epoll_ctl failed.\n");
        exit(EXIT_FAILURE);
    }
    printf("ADDED EVENT\n");
    return;
}

inline int network_epoll_wait(socket_t epollfd, struct epoll_event *events, int maxevents, int timeout) {
    int nfds = epoll_wait(epollfd, events, maxevents, timeout);
    if (nfds < 0) {
        printf("epoll_wait failed.%s\n", strerror(errno));
        exit(EXIT_FAILURE);
    }
    return nfds;
}

void network_epoll_close(socket_t epollfd) {
    close(epollfd);
}

inline void network_close(socket_t socket) {
    close(socket);
}

static inline FILE* network_load_file(const char *filename) {
    return fopen(filename, "rb");
}
#endif

#ifdef __cplusplus
}
#endif

#endif //NETWORK_H
