/**
 * Created by Uthowaipru Chowdhury Baiching on 10/15/2025.
 * This is the lowest layer that handles raw socket operations
*/

#ifndef NETWORK_H
#define NETWORK_H

#include <stdint.h>

// platform detection
#if defined(_WIN32)
#define NETWORK_USE_WIN32_IMPL
#elif defined(__linux__) || defined(__sun) || defined(__FreeBSD__) || defined(__NetBSD__) || defined(__OpenBSD__) || (defined(__APPLE__) && defined(__MACH__)) || defined(__MSYS__) || defined(__unix__)
#define NETWORK_USE_POSIX_IMPL
#else
#pragma message("Warning: Unknown OS")
#define NETWORK_USE_POSIX_IMPL
#endif

#ifdef __cplusplus
extern "C" {
#endif

// some platform specific definitions
#if defined(NETWORK_USE_WIN32_IMPL)
#ifdef _WINSOCKAPI_
#error winsock.h included instead of winsock2.h
#endif
#ifdef __MINGW32__ // mingw automatically sends warnings
#define _WINSOCKAPI_ // prevents the inclusion of winsock.h in windows.h
#endif
#include <winsock2.h>
#include <windows.h>
typedef SOCKET socket_t;
#elif defined(NETWORK_USE_POSIX_IMPL)
typedef int socket_t;
#endif





#ifdef __cplusplus
    }
#endif


#endif //NETWORK_H
