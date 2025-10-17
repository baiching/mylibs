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


#ifdef __cplusplus
}
#endif




#endif //NETWORK_H
