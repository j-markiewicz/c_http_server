/* Cross-platform-ish sockets with support for basic operations on Windows and
 * Linux (and maybe more).
 */

#ifndef C_HTTP_SERVER_SOCKET_H
#define C_HTTP_SERVER_SOCKET_H

#include <stdbool.h>
#include <stdint.h>
#include <stddef.h>

#ifdef _WIN32

#include <winsock2.h>
#include <Ws2tcpip.h>

#else
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/tcp.h>
#endif

#ifdef _WIN32
typedef SOCKET Socket;
#else
typedef int32_t Socket;
#endif

/* The default Buffer capacity */
#define SERV_DEFAULT_BUFFER_CAP 2048

/* Close the provided socket. Any error will be ignored. */
void close_socket(Socket sock);

/* Create a new dual-stack (IPv4 and IPv6) TCP socket using the provided port.
 * The returned socket will be ready to accept new connections. If an error
 * occurs, this function will stop the server.
 */
Socket create_socket(uint16_t listen_port);

/* Accept an incoming connection on a socket. Returns true if the connection
 * is accepted, false if an error occurs. The connection can be used via the
 * `incoming` `Socket`, which on success will contain the socket for the new
 * connection, ready to be used with `send`, `recv`, etc.
 */
bool accept_connection(Socket sock, Socket* incoming);

/* A byte buffer with known length and capacity. The internal buffer `buf` is a
 * heap-allocated array, which should be `free`d after use
 */
struct Buffer {
	size_t len;
	size_t cap;
	uint8_t* buf;
};

/* Create a new buffer with length 0 and capacity `cap` (or a default capacity
 * if `cap` is 0). The buffer should be freed with `free_buffer` after use.
 */
struct Buffer new_buffer(size_t cap);

/* Free the given buffer */
void free_buffer(struct Buffer buf);

/* Convert the given buffer to a C string, shrinking the allocation to `buf.len`
 * if possible. The contents of the buffer are returned as-is, but with the
 * addition of a null terminator. If the buffer is full and can not be
 * reallocated, the last byte is overwritten with the null terminator. The
 * newly returned string should be `free`d after use, but the original buf is
 * consumed by this function and must not be used after calling this function.
 */
char* buffer_to_str(struct Buffer buf);

/* Receive up to `buf.cap` bytes into the provided buffer from the given
 * socket. On success, this returns true and `buf` will hold `buf.len`
 * received bytes. On failure, false is returned and `buf` will have length 0.
 * If no data is received (e.g. because the socket is closed), but there weren't
 * any errors, true is returned, but `buf.len` will be 0.
 */
bool receive(Socket sock, struct Buffer* buf);

#endif
