/***
 *This API is not a general purpose TCP API, it's exclusively designed for file transfer
 */

#ifndef NETWORK_H
#define NETWORK_H
#include <stdio.h>

// platform detection
#ifdef _WIN32
#include <winsock2.h>
#include <ws2tcpip.h>
typedef SOCKET socket_t;
#else
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <errno.h>
typedef int socket_t;
#endif

#ifdef __cplusplus
extern "C" {
#endif

// socket
static socket_t GLOBAL_SOCKET = INVALID_SOCKET;

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

    // Connection errors
    NETWORK_ERROR_CONNECTION_REFUSED = -1,
    NETWORK_ERROR_TIMEOUT = -2,
    NETWORK_ERROR_HOST_UNREACHABLE = -3,
    NETWORK_ERROR_DNS_FAILURE = -4,

    // Socket errors
    NETWORK_ERROR_ADDR_IN_USE = -10,
    NETWORK_ERROR_PERMISSION_DENIED = -11,
    NETWORK_ERROR_INVALID_SOCKET = -12,

    // System errors
    NETWORK_ERROR_PLATFORM_INIT_FAILED = -20,
    NETWORK_ERROR_OUT_OF_MEMORY = -21,

    // Parameter errors
    NETWORK_ERROR_INVALID_ADDRESS = -30,
    NETWORK_ERROR_INVALID_PORT = -31
} network_result;

// core cycle
int network_init(void);
void network_cleanup(void);

// server side
/**
 * @brief It makes the server starts listening. it automatically takes care of socket creation, bind and listen
 * @param port a port address from user
 * @return
 *   NETWORK_SUCCESS = 0 : if the server is successfully listening it will return this flag
 *   SOCKET_CREATE_FAILED = 1: socket failed during creation
 *   SOCKET_BIND_FAILED = 2: failed during bind
 *   SOCKET_LISTEN_FAILED = 3: failed to start listening
 */
network_result network_listen(int port);

/**
 * @brief This is a special function where user enters specific ip for server to listen
 * @param ip a string ip from user
 * @param port a port address from user
 * @return
 *   NETWORK_SUCCESS = 0 : if the server is successfully listening it will return this flag
 *   SOCKET_CREATE_FAILED = 1: socket failed during creation
 *   SOCKET_BIND_FAILED = 2: failed during bind
 *   SOCKET_LISTEN_FAILED = 3: failed to start listening
 *
 * @note it automatically takes care of socket creation, bind and listen
 */
network_result network_listen_on(const char *ip, int port); // specific interface

/**
 * @brief it accepts connection requests from client
 * 
 * This function extracts the first connection request from the queue of 
 * pending connections for the listening socket, creates a new connected 
 * socket and returns a file descriptor for that socket.
 *
 * @param client_data : expects an empty struct that'll be populated with clients data after system call accept()
 * @return
 *      -1 : upon failure it will return this value
 *      socket_t: variable or file descriptor of new socket from client will be returned
 */
socket_t network_accept(struct sockaddr_in *client_data); // accept connection

// client side
network_result network_connect(const char *ip, int port);
network_result network_connect_timeout(const char *ip, int port, int timeout_ms);

// data transfer
int network_send(socket_t socket, const void *data, size_t size);
int network_recv(socket_t socket, void *data, size_t size);

// closing socket
void network_close(socket_t socket);

// Utilities
int network_set_nonblocking(socket_t sock);  // TODO : later
int network_would_block(void);               // TODO : later


#ifdef NETWORK_IMPLEMENTATION

#ifdef _WIN32
    static WSADATA wsaData;
#endif

inline int network_init(void) {
#ifdef _WIN32
    return WSAStartup(MAKEWORD(2, 2), &wsa_data);
#else
    return 0;
#endif
}

inline void network_cleanup(void) {
#ifdef _WIN32
    WSACleanup();
#else
    return 0;
#endif
}

inline network_result network_listen(int port) {
        socket_t server_fd = socket(AF_INET, SOCK_STREAM, 0);
        if (server_fd == INVALID_SOCKET) {
            printf("Socket creation failed.\n");
            return SOCKET_CREATE_FAILED;
        }
        struct sockaddr_in server_address;
        server_address.sin_family = AF_INET;
        server_address.sin_port = htons(port);
        server_address.sin_addr.s_addr = INADDR_ANY;

    // bind the address to the given ip and port above
    if (bind(server_fd, (struct sockaddr *)&server_address, sizeof(server_address)) < 0) {
        #ifdef _WIN32
                int err = WSAGetLastError();
                switch (err) {
                    case WSAEADDRINUSE:
                        printf("Port %d is already in use.\n", port);
                        break;
                    case WSAEACCES:
                        printf("Port %d is denied.\n", port);
                        break;
                    default:
                        printf("Port %d is unavailable.\n", port);
                }
        network_close(server_fd);
        #else
                if (errno == EADDRINUSE) {
                    printf("Port %d is already in use.\n", port);
                }
                else if (errno ==EACCES) {
                    printf("Port %d is denied.\n", port);
                }
        network_close(server_fd);
        #endif

        return SOCKET_BIND_FAILED;
    }

    // SOMAXCONN : maximum baklog by the system
    if (listen(server_fd, SOMAXCONN) < 0) {
        #ifdef _WIN32
        int err = WSAGetLastError();
        switch (err) {
            case WSAEINVAL:          // Socket not bound or already listening
                printf("Socket not bound or already listening\n");
                break;
            case WSAENOTCONN:        // Socket not connected (for connection-oriented)
                printf("Socket is not connected\n");
                break;
            case WSAEADDRINUSE:      // Address already in use
                printf("Address already in use\n");
                break;
            case WSAEISCONN:         // Socket already connected
                printf("Socket is already connected\n");
                break;
            case WSAEMFILE:          // No more socket descriptors available
                printf("No more socket descriptors available\n");
                break;
            case WSAENOBUFS:         // No buffer space available
                printf("No buffer space available\n");
                break;
            case WSAENOTSOCK:        // Not a socket
                printf("Descriptor is not a socket\n");
                break;
            default:
                printf("listen failed with error: %d\n", err);

        }
        #else
        switch (errno) {
            case EADDRINUSE:  // Address already in use
                printf("Another socket is already listening on same port\n");
                break;
            case EBADF:       // Invalid socket descriptor
                printf("Invalid socket file descriptor\n");
                break;
            case ENOTSOCK:    // Not a socket
                printf("File descriptor is not a socket\n");
                break;
            case EOPNOTSUPP:  // Socket doesn't support listening
                printf("Socket type does not support listening\n");
                break;
            case EINVAL:      // Socket already connected or not bound
                printf("Socket is already connected or not properly bound\n");
                break;
            default:
                perror("listen failed");
        }

        #endif
        network_close(server_fd);
        return SOCKET_LISTEN_FAILED;
    }
    GLOBAL_SOCKET = server_fd;
    return NETWORK_SUCCESS;
}

inline network_result network_listen_on(const char *ip, int port) {
    socket_t server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd == INVALID_SOCKET) {
        printf("Socket creation failed.\n");
        return SOCKET_CREATE_FAILED;
    }
    struct sockaddr_in server_address;
    server_address.sin_family = AF_INET;
    server_address.sin_port = htons(port);
    server_address.sin_addr.s_addr = inet_addr(ip);

    // bind the address to the given ip and port above
    if (bind(server_fd, (struct sockaddr *)&server_address, sizeof(server_address)) < 0) {
        #ifdef _WIN32
                int err = WSAGetLastError();
                switch (err) {
                    case WSAEADDRINUSE:
                        printf("IP:Port %s:%d is already in use.\n", ip, port);
                        break;
                    case WSAEACCES:
                        printf("Port %d is denied.\n", port);
                        break;
                    default:
                        printf("Port %d is unavailable.\n", port);
                }
        network_close(server_fd);
        #else
                if (errno == EADDRINUSE) {
                    printf("Port %d is already in use.\n", port);
                }
                else if (errno ==EACCES) {
                    printf("Port %d is denied.\n", port);
                }
        network_close(server_fd);
        #endif

        return SOCKET_BIND_FAILED;
    }

    // SOMAXCONN : maximum baklog by the system
    if (listen(server_fd, SOMAXCONN) < 0) {
        #ifdef _WIN32
        int err = WSAGetLastError();
        switch (err) {
            case WSAEINVAL:          // Socket not bound or already listening
                printf("Socket not bound or already listening\n");
                break;
            case WSAENOTCONN:        // Socket not connected (for connection-oriented)
                printf("Socket is not connected\n");
                break;
            case WSAEADDRINUSE:      // Address already in use
                printf("Address already in use\n");
                break;
            case WSAEISCONN:         // Socket already connected
                printf("Socket is already connected\n");
                break;
            case WSAEMFILE:          // No more socket descriptors available
                printf("No more socket descriptors available\n");
                break;
            case WSAENOBUFS:         // No buffer space available
                printf("No buffer space available\n");
                break;
            case WSAENOTSOCK:        // Not a socket
                printf("Descriptor is not a socket\n");
                break;
            default:
                printf("listen failed with error: %d\n", err);

        }
        #else
        switch (errno) {
            case EADDRINUSE:  // Address already in use
                printf("Another socket is already listening on same port\n");
                break;
            case EBADF:       // Invalid socket descriptor
                printf("Invalid socket file descriptor\n");
                break;
            case ENOTSOCK:    // Not a socket
                printf("File descriptor is not a socket\n");
                break;
            case EOPNOTSUPP:  // Socket doesn't support listening
                printf("Socket type does not support listening\n");
                break;
            case EINVAL:      // Socket already connected or not bound
                printf("Socket is already connected or not properly bound\n");
                break;
            default:
                perror("listen failed");
        }

        #endif
        network_close(server_fd);
        return SOCKET_LISTEN_FAILED;
    }
    GLOBAL_SOCKET = server_fd;
    return NETWORK_SUCCESS;
}

inline socket_t network_accept(struct sockaddr_in *client_data) {
    // TODO
    socklen_t client_len = sizeof(struct sockaddr_in);
    socket_t client_socket = accept(GLOBAL_SOCKET, (struct sockaddr *) client_data, &client_len);

    if(client_socket == INVALID_SOCKET) {
        #ifdef _WIN32
        int err = WSAGetLastError();

        switch(err) {
            case WSANOTINITIALISED:
                printf("A successful WSAStartup call must occur before using this function.\n");
                break;
            case WSAECONNRESET:
                printf("An incoming connection was indicated, but was subsequently terminated by the remote peer prior to accepting the call.\n");
                break;
            case WSAEFAULT:
                printf("The addrlen parameter is too small or addr is not a valid part of the user address space.\n");
                break;
            cae WSAEINTR:
                printf("A blocking Windows Sockets 1.1 call was canceled through WSACancelBlockingCall.\n");
                break;
            case WSAEINVAL:
                printf("The listen function was not invoked prior to accept.\n");
                break;
            case WSAEINPROGRESS:
                printf("A blocking Windows Sockets 1.1 call is in progress, or the service provider is still processing a callback function.\n");
                break;
            case WSAEMFILE:
                printf("The queue is nonempty upon entry to accept and there are no descriptors available.\n");
                break;
            case WSAENETDOWN:
                printf("The network subsystem has failed.\n");
                break;
            case WSAENOBUFS:
                printf("No buffer space is available.\n");
                break;
            case WSAENOTSOCK:
                printf("The descriptor is not a socket.\n");
                break;
            case WSAEOPNOTSUPP:
                printf("The referenced socket is not a type that supports connection-oriented service.\n");
                break;
            case WSAEWOULDBLOCK:
                printf("The socket is marked as nonblocking and no connections are present to be accepted.\n");
                break;
            default:
                printf("Unknown error code: %d\n", error_code);
                break;

        }

        #else
        switch(errno) {
            case EAGAIN:
                printf("The socket is marked nonblocking and no connections are present to be accepted.\n");
                break;
            case EBADF:
                printf("sockfd is not an open file descriptor.\n");
                break;
            case ECONNABORTED:
                printf("A connection has been aborted.\n");
                break;
            case EFAULT:
                printf("The addr argument is not in a writable part of the user address space.\n");
                break;
            case EINTR:
                printf("The system call was interrupted by a signal that was caught before a valid connection arrived.\n");
                break;
            case EINVAL:
                printf("Socket is not listening for connections, addrlen is invalid, or (accept4()) invalid value in flags.\n");
                break;
            case EMFILE:
                printf("The per-process limit on the number of open file descriptors has been reached.\n");
                break;
            case ENFILE:
                printf("The system-wide limit on the total number of open files has been reached.\n");
                break;
            case ENOMEM:
                printf("Not enough free memory. This often means that the memory allocation is limited by the socket buffer limits.\n");
                break;
            case ENOTSOCK:
                printf("The file descriptor sockfd does not refer to a socket.\n");
                break;
            case EOPNOTSUPP:
                printf("The referenced socket is not of type SOCK_STREAM.\n");
                break;
            case EPERM:
                printf("Firewall rules forbid connection.\n");
                break;
            case EPROTO:
                printf("Protocol error.\n");
                break;
            default:
                printf("Unknown error code: %d\n", error_code);
                break;
        }
        return -1;
    }
    return client_socket;
}

inline void network_close(socket_t socket) {
#ifdef _WIN32
    closesocket(socket);
#else
    close(socket);
#endif
}
#endif

#ifdef __cplusplus
}
#endif

#endif //NETWORK_H
