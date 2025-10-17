/**
 * Created by Uthowaipru Chowdhury Baiching on 10/15/2025.
 * This is the lowest layer that handles raw socket operations
*/

#ifndef NETWORK_H
#define NETWORK_H

// platform detection
#ifdef _WIN32
#include <winsock2.h>
#include <ws2tcpip.h>
typedef SOCKET socket_t;
#else
#include <sys/socket.h>
#include <netinet/in.h>
typedef int socket_t;
#endif

#ifdef __cplusplus
extern "C" {
#endif

// return codes
typedef enum {
    NETWORK_SUCCESS = 0,

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
network_result network_listen(int port);
network_result network_listen_on(const char *ip, int port); // specific interface
network_result network_accept(socket_t socket); // accept connection

// client side
network_result network_connect(const char *ip, int port);
network_result network_connect_timeout(const char *ip, int port, int timeout_ms);

// data transfer
int network_send(socket_t socket, const void *data, size_t size);
int network_recv(socket_t socket, void *data, size_t size);

// cleanup
void network_close(socket_t socket);

// Utilities
int network_set_nonblocking(socket_t sock);  // TODO : later
int network_would_block(void);               // TODO : later

#ifdef __cplusplus
}
#endif




#endif //NETWORK_H
