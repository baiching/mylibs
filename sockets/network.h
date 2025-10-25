/**
 * @file network.h
 * @brief Minimal Linux Socket Library - Lightweight TCP networking for Linux
 * @version 1.0.0
 * 
 * Header-only socket library providing clean TCP networking APIs for Linux.
 * 
 * Available APIs:
 * Server:
 *   • network_listen()       - Listen on port (all interfaces)
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
 * 
 */

#ifndef NETWORK_H
#define NETWORK_H

#include <stdio.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <errno.h>
#include <unistd.h>
#include <string.h>
#include <netdb.h>

typedef int socket_t;

#ifdef __cplusplus
extern "C" {
#endif


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

// core cycle
int network_init(void);
void network_cleanup(void);

// server side
/**
 * @brief Creates a TCP server socket that listens for incoming connections on the specified port.
 *
 * @param port Null-terminated string specifying the port number or service name (e.g., "8080", "http")
 * @return socket_t On success: valid socket descriptor for accepting connections
 *                  On failure: -1
 *
 * @details This function creates a TCP server socket bound to all available network interfaces
 * (both IPv4 and IPv6 if supported) and starts listening for incoming connections.
 *
 * The function performs the following operations:
 * - Initializes address hints for passive TCP socket
 * - Resolves local addresses using getaddrinfo() with AI_PASSIVE flag
 * - Creates a socket with the appropriate address family (IPv4/IPv6)
 * - Binds the socket to the specified port on all interfaces
 * - Starts listening for incoming connections with a backlog of 10
 * - Cleans up allocated resources before returning
 *
 * @note The server listens on all available network interfaces (0.0.0.0 for IPv4, :: for IPv6).
 * @note The backlog queue size is set to 10 pending connections (see BACKLOG macro).
 * @note Memory allocated by getaddrinfo() is properly freed before return.
 * @note On failure, appropriate error messages are printed to stdout.
 *
 * @warning The returned socket must be closed using network_close() when no longer needed.
 * @warning The port parameter must be a valid null-terminated string.
 *
 * @see network_accept()
 * @see network_close()
 * @see BACKLOG
 *
 * @code
 * // Example: Create a server listening on port 8080
 * socket_t server_sock = network_listen("8080");
 * if (server_sock >= 0) {
 *     printf("Server listening on port 8080\n");
 *
 *     // Accept incoming connections
 *     socket_t client_sock = network_accept(server_sock, NULL);
 *     if (client_sock >= 0) {
 *         // Handle client connection
 *         network_close(client_sock);
 *     }
 *     network_close(server_sock);
 * }
 * @endcode
 *
 * @code
 * // Example: Create a server using service name
 * socket_t server_sock = network_listen("http");
 * if (server_sock >= 0) {
 *     printf("Server listening on HTTP port (80)\n");
 *     // ... use server socket
 *     network_close(server_sock);
 * }
 * @endcode
 */
socket_t network_listen(const char *port);

/**
 * @brief Creates a TCP server socket that listens for incoming connections on a specific IP address and port.
 *
 * @param ip Null-terminated string specifying the IP address to bind to (e.g., "192.168.1.100", "::1")
 * @param port Null-terminated string specifying the port number or service name (e.g., "8080", "http")
 * @return socket_t On success: valid socket descriptor for accepting connections
 *                  On failure: -1
 *
 * @details This function creates a TCP server socket bound to a specific network interface
 * and starts listening for incoming connections. Unlike network_listen(), this function
 * allows binding to a specific IP address rather than all available interfaces.
 *
 * The function performs the following operations:
 * - Initializes address hints for passive TCP socket
 * - Resolves specific address using getaddrinfo() with AI_PASSIVE flag
 * - Creates a socket with the appropriate address family (IPv4/IPv6)
 * - Binds the socket to the specified IP address and port
 * - Starts listening for incoming connections with a backlog of 10
 * - Cleans up allocated resources before returning
 *
 * @note Use this function when you need to listen on a specific network interface.
 * @note The backlog queue size is set to 10 pending connections (see BACKLOG macro).
 * @note Memory allocated by getaddrinfo() is properly freed before return.
 * @note On failure, appropriate error messages are printed to stdout.
 *
 * @warning The returned socket must be closed using network_close() when no longer needed.
 * @warning Both ip and port parameters must be valid null-terminated strings.
 *
 * @see network_listen()
 * @see network_accept()
 * @see network_close()
 * @see BACKLOG
 *
 * @code
 * // Example: Create a server listening on specific IP and port
 * socket_t server_sock = network_listen_on("192.168.1.100", "8080");
 * if (server_sock >= 0) {
 *     printf("Server listening on 192.168.1.100:8080\n");
 *
 *     // Accept incoming connections
 *     socket_t client_sock = network_accept(server_sock, NULL);
 *     if (client_sock >= 0) {
 *         // Handle client connection
 *         network_close(client_sock);
 *     }
 *     network_close(server_sock);
 * }
 * @endcode
 *
 * @code
 * // Example: Create an IPv6 server on localhost
 * socket_t server_sock = network_listen_on("::1", "8080");
 * if (server_sock >= 0) {
 *     printf("IPv6 server listening on [::1]:8080\n");
 *     // ... use server socket
 *     network_close(server_sock);
 * }
 * @endcode
 *
 * @code
 * // Example: Listen on all IPv4 interfaces (alternative to network_listen)
 * socket_t server_sock = network_listen_on("0.0.0.0", "8080");
 * if (server_sock >= 0) {
 *     printf("Server listening on all IPv4 interfaces port 8080\n");
 *     // ... use server socket
 *     network_close(server_sock);
 * }
 * @endcode
 */
socket_t network_listen_on(const char *ip, const char *port); // specific interface

/**
 * @brief Accepts an incoming connection on a listening server socket.
 *
 * @param sockfd Listening socket descriptor returned by network_listen() or network_listen_on()
 * @param client_storage Pointer to sockaddr_storage structure to receive client address information
 * @return socket_t On success: new socket descriptor for communicating with the client
 *                  On failure: -1
 *
 * @details This function extracts the first connection request from the queue of pending connections
 * for the listening socket, creates a new connected socket, and returns a file descriptor for
 * that socket. The new socket is used for communication with the client, while the original
 * listening socket remains open to accept additional connections.
 *
 * The client's address information (IP address and port) is stored in the provided
 * sockaddr_storage structure, which can be used to identify the connecting client.
 *
 * @note This function blocks until a connection is available, unless the socket is set to non-blocking mode.
 * @note The original listening socket (sockfd) remains open and should continue to be used for accepting new connections.
 * @note The new socket (return value) is dedicated to the specific client and should be closed when the connection ends.
 * @note On successful connection, "Client connected." is printed to stdout.
 *
 * @warning The sockfd parameter must be a valid socket descriptor from network_listen() or network_listen_on().
 * @warning The client_storage parameter must point to a valid sockaddr_storage structure.
 * @warning The returned client socket must be closed using network_close() when the connection ends.
 *
 * @see network_listen()
 * @see network_listen_on()
 * @see network_close()
 * @see network_send()
 * @see network_recv()
 *
 * @code
 * // Example: Accept connections in a server loop
 * socket_t server_sock = network_listen("8080");
 * if (server_sock >= 0) {
 *     printf("Server listening on port 8080\n");
 *
 *     while (1) {
 *         struct sockaddr_storage client_addr;
 *         socket_t client_sock = network_accept(server_sock, &client_addr);
 *
 *         if (client_sock >= 0) {
 *             // Handle client connection in new thread or process
 *             char buffer[1024];
 *             int bytes = network_recv(client_sock, buffer, sizeof(buffer));
 *             if (bytes > 0) {
 *                 network_send(client_sock, "Message received");
 *             }
 *             network_close(client_sock);
 *         }
 *     }
 *     network_close(server_sock);
 * }
 * @endcode
 *
 * @code
 * // Example: Get client address information
 * struct sockaddr_storage client_addr;
 * socket_t client_sock = network_accept(server_sock, &client_addr);
 *
 * if (client_sock >= 0) {
 *     char client_ip[INET6_ADDRSTRLEN];
 *     if (client_addr.ss_family == AF_INET) {
 *         struct sockaddr_in *s = (struct sockaddr_in *)&client_addr;
 *         inet_ntop(AF_INET, &s->sin_addr, client_ip, sizeof(client_ip));
 *     } else {
 *         struct sockaddr_in6 *s = (struct sockaddr_in6 *)&client_addr;
 *         inet_ntop(AF_INET6, &s->sin6_addr, client_ip, sizeof(client_ip));
 *     }
 *     printf("Client connected from: %s\n", client_ip);
 * }
 * @endcode
 *
 * @code
 * // Example: Accept without storing client address (if not needed)
 * socket_t client_sock = network_accept(server_sock, NULL);
 * if (client_sock >= 0) {
 *     // Client connected, but we don't care about their address
 *     // ... handle connection
 *     network_close(client_sock);
 * }
 * @endcode
 */
socket_t network_accept(socket_t socktfd, struct sockaddr_storage *client_storage); // accept connection

// client side

/**
 * @brief Establishes a connection to a remote server using pre-resolved address information.
 *
 * @param server_address Pointer to addrinfo structure containing pre-resolved server address information
 * @return socket_t On success: valid socket descriptor for communicating with the server
 *                  On failure: -1
 *
 * @details This function creates a client socket and establishes a TCP connection to a remote server
 * using pre-resolved address information from getaddrinfo(). The server_address parameter should
 * contain the complete address information including family, socket type, protocol and address data.
 *
 * The function performs the following operations:
 * - Creates a socket matching the server address family and type
 * - Attempts to connect to the remote server using the provided address information
 * - Returns the connected socket on success, or cleans up and returns -1 on failure
 *
 * @note This function is typically used with address information obtained from getaddrinfo().
 * @note The function blocks until the connection is established or fails.
 * @note On successful connection, "Socket successfully connected." is printed to stdout.
 * @note The returned socket must be closed with network_close() when no longer needed.
 *
 * @warning The server_address parameter must be a valid addrinfo structure from getaddrinfo().
 * @warning The server_address should contain complete and valid address information.
 * @warning The returned socket descriptor must be closed using network_close().
 *
 * @see getaddrinfo()
 * @see network_close()
 * @see network_send()
 * @see network_recv()
 *
 * @code
 * // Example: Connect to a server using getaddrinfo
 * struct addrinfo hints, *res;
 * memset(&hints, 0, sizeof(hints));
 * hints.ai_family = AF_UNSPEC;
 * hints.ai_socktype = SOCK_STREAM;
 *
 * if (getaddrinfo("example.com", "http", &hints, &res) == 0) {
 *     socket_t sock = network_connect(res);
 *     if (sock >= 0) {
 *         // Successfully connected to example.com:80
 *         network_send(sock, "GET / HTTP/1.0\r\n\r\n");
 *         // ... receive response
 *         network_close(sock);
 *     }
 *     freeaddrinfo(res);
 * }
 * @endcode
 *
 * @code
 * // Example: Connect to specific IP and port
 * struct addrinfo hints, *res;
 * memset(&hints, 0, sizeof(hints));
 * hints.ai_family = AF_UNSPEC;
 * hints.ai_socktype = SOCK_STREAM;
 *
 * if (getaddrinfo("192.168.1.100", "8080", &hints, &res) == 0) {
 *     socket_t sock = network_connect(res);
 *     if (sock >= 0) {
 *         printf("Connected to server at 192.168.1.100:8080\n");
 *         // ... communicate with server
 *         network_close(sock);
 *     }
 *     freeaddrinfo(res);
 * }
 * @endcode
 *
 * @code
 * // Example: Try multiple addresses until one succeeds
 * struct addrinfo hints, *res, *p;
 * memset(&hints, 0, sizeof(hints));
 * hints.ai_family = AF_UNSPEC;
 * hints.ai_socktype = SOCK_STREAM;
 *
 * if (getaddrinfo("example.com", "http", &hints, &res) == 0) {
 *     for (p = res; p != NULL; p = p->ai_next) {
 *         socket_t sock = network_connect(p);
 *         if (sock >= 0) {
 *             printf("Connected using address family: %d\n", p->ai_family);
 *             // ... use the connection
 *             network_close(sock);
 *             break;
 *         }
 *     }
 *     freeaddrinfo(res);
 * }
 * @endcode
 */
socket_t network_connect(const struct addrinfo *server_address);
network_result network_connect_timeout(const char *ip, int port, int timeout_ms);

// data transfer

/**
 * @brief Sends a null-terminated string over a connected socket.
 *
 * @param socketfd Connected socket descriptor for sending data
 * @param data Null-terminated string to send over the socket
 * @return int On success: number of bytes actually sent
 *             On failure: -1\n
 *             On empty string: 0 (not an error)
 *
 * @details This function sends a null-terminated string over an established socket connection.
 * The function automatically calculates the string length using strlen() and sends the entire
 * content (excluding the null terminator) to the remote endpoint.
 *
 * The function performs the following operations:
 * - Validates the socket descriptor and data pointer
 * - Calculates the string length using strlen()
 * - Sends the data using the send() system call with flags=0
 * - Returns the actual number of bytes sent
 *
 * @note This function is designed for sending null-terminated strings only.
 * @note For binary data or explicit length control, consider a different API.
 * @note The return value may be less than the string length due to partial sends (normal TCP behavior).
 * @note Sending an empty string returns 0 (not an error).
 * @note The function uses blocking I/O by default (flags=0).
 *
 * @warning The socket must be in a connected state (established TCP connection).
 * @warning The data parameter must be a valid null-terminated string.
 * @warning Partial sends are normal TCP behavior and not treated as errors.
 *
 * @see network_recv()
 * @see network_connect()
 * @see network_accept()
 *
 * @code
 * // Example: Send a simple message
 * socket_t sock = network_connect(server_addr);
 * if (sock >= 0) {
 *     int bytes_sent = network_send(sock, "Hello Server!");
 *     if (bytes_sent > 0) {
 *         printf("Sent %d bytes to server\n", bytes_sent);
 *     }
 *     network_close(sock);
 * }
 * @endcode
 *
 * @code
 * // Example: Send multiple messages in a chat application
 * const char *messages[] = {"USER Alice", "JOIN #general", "MSG Hello everyone!"};
 *
 * for (int i = 0; i < 3; i++) {
 *     int bytes = network_send(sock, messages[i]);
 *     if (bytes < 0) {
 *         printf("Failed to send message: %s\n", messages[i]);
 *         break;
 *     }
 * }
 * @endcode
 *
 * @code
 * // Example: Handle partial sends (though rare in practice)
 * const char *long_message = "This is a very long message...";
 * int bytes_sent = network_send(sock, long_message);
 *
 * if (bytes_sent >= 0) {
 *     if (bytes_sent < strlen(long_message)) {
 *         printf("Partial send: %d of %zu bytes sent\n",
 *                bytes_sent, strlen(long_message));
 *         // In production, you might want to resend remaining data
 *     } else {
 *         printf("Full message sent successfully\n");
 *     }
 * }
 * @endcode
 *
 * @code
 * // Example: Send empty string (edge case)
 * int bytes = network_send(sock, "");
 * if (bytes == 0) {
 *     printf("Empty string sent (not an error)\n");
 * }
 * @endcode
 */
socket_t network_send(socket_t socket, const void *data);

/**
 * @brief Receives data from a connected socket into a provided buffer.
 *
 * @param socketfd Connected socket descriptor for receiving data
 * @param data Pointer to buffer where received data will be stored
 * @param buffer_size Maximum number of bytes that can be stored in the buffer
 * @return int On success: number of bytes actually received (0 indicates connection closed)
 *             On failure: -1
 *
 * @details This function receives data from an established socket connection and stores it
 * in the provided buffer. The function will receive up to buffer_size bytes, but may return
 * fewer bytes if less data is currently available.
 *
 * The function performs the following operations:
 * - Validates the socket descriptor, buffer pointer, and buffer size
 * - Receives available data using the recv() system call with flags=0
 * - Returns the actual number of bytes received
 *
 * @note The received data is stored as raw bytes - no null terminator is added automatically.
 * @note A return value of 0 indicates the connection has been closed gracefully by the remote peer.
 * @note The return value may be less than buffer_size due to partial receives (normal TCP behavior).
 * @note The function uses blocking I/O by default (will wait until data is available or connection closes).
 * @note For string data, the caller must add a null terminator if needed.
 *
 * @warning The socket must be in a connected state (established TCP connection).
 * @warning The data buffer must be writable and large enough to hold received data.
 * @warning The received data is not null-terminated - caller must handle this if working with strings.
 * @warning Partial receives are normal TCP behavior and not treated as errors.
 *
 * @see network_send()
 * @see network_connect()
 * @see network_accept()
 *
 * @code
 * // Example: Receive data into a buffer
 * char buffer[1024];
 * int bytes_received = network_recv(sock, buffer, sizeof(buffer));
 *
 * if (bytes_received > 0) {
 *     // Add null terminator for string processing
 *     buffer[bytes_received] = '\0';
 *     printf("Received %d bytes: %s\n", bytes_received, buffer);
 * } else if (bytes_received == 0) {
 *     printf("Connection closed by remote peer\n");
 * } else {
 *     printf("Receive error\n");
 * }
 * @endcode
 *
 * @code
 * // Example: Receive loop for complete message
 * char buffer[1024];
 * int total_received = 0;
 *
 * while (total_received < sizeof(buffer) - 1) {
 *     int bytes = network_recv(sock, buffer + total_received,
 *                             sizeof(buffer) - total_received);
 *     if (bytes <= 0) break;
 *     total_received += bytes;
 *
 *     // Check if we have a complete message (example: newline terminated)
 *     if (buffer[total_received - 1] == '\n') {
 *         buffer[total_received] = '\0';
 *         printf("Complete message: %s", buffer);
 *         break;
 *     }
 * }
 * @endcode
 *
 * @code
 * // Example: Handle connection closure
 * char buffer[256];
 * while (1) {
 *     int bytes = network_recv(sock, buffer, sizeof(buffer));
 *     if (bytes > 0) {
 *         // Process received data
 *         buffer[bytes] = '\0';
 *         printf("Received: %s", buffer);
 *     } else if (bytes == 0) {
 *         printf("Server closed the connection\n");
 *         break;
 *     } else {
 *         printf("Receive error, closing connection\n");
 *         break;
 *     }
 * }
 * @endcode
 *
 * @code
 * // Example: Binary data reception
 * uint8_t binary_data[512];
 * int bytes = network_recv(sock, binary_data, sizeof(binary_data));
 * if (bytes > 0) {
 *     // Process binary data (no null terminator needed)
 *     printf("Received %d bytes of binary data\n", bytes);
 * }
 * @endcode
 */
socket_t network_recv(socket_t socketfd, void *data, size_t buffer_size);

// closing socket
void network_close(socket_t socket);

// Utilities
int network_set_nonblocking(socket_t sock);  // TODO : later
int network_would_block(void);               // TODO : later


#ifdef NETWORK_IMPLEMENTATION

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

inline socket_t network_connect(const struct addrinfo *server_address) {
    if (server_address == NULL) {
        printf("Address is NULL.\n");
        return -1;
    }

    const socket_t client_socket = socket(server_address->ai_family, server_address->ai_socktype, server_address->ai_protocol);
    if (client_socket < 0) {
        printf("Socket creation failed.\n");
        return -1;
    }

    if (connect(client_socket, server_address->ai_addr, server_address->ai_addrlen) == 0) {
        printf("Socket successfully connected.\n");
        return client_socket;
    }
    else {
        printf("Connection failed. %s\n", strerror(errno));
        network_close(client_socket);
        return -1;
    }
}

inline socket_t network_send(socket_t socketfd, const void *data) {
    if (socketfd < 0) {
        printf("Invalid socket descriptor: %d\n", socketfd);
        return -1;
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
        return -1;
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


inline void network_close(socket_t socket) {
    close(socket);
}
#endif

#ifdef __cplusplus
}
#endif

#endif //NETWORK_H
