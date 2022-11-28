/* Implementation of `socket.h`, see that file for documentation and types */

#include "socket.h"

#include <stdio.h>
#include <unistd.h>
#include <memory.h>
#include <stdlib.h>

#include "log.h"
#include "misc.h"

void close_socket(Socket sock) {
	#ifdef _WIN32
	int32_t status = shutdown(sock, SD_BOTH);
	if (status == 0) { closesocket(sock); }
	#else
	int32_t status = shutdown(sock, SHUT_RDWR);
	if (status == 0) { close(sock); }
	#endif
}

Socket create_socket(uint16_t listen_port) {
	#ifdef _WIN32
	/* Initialize Windows Sockets version 2.2. This is not required on Linux. */
	struct WSAData wsa_data;
	int32_t wsa_res = WSAStartup(MAKEWORD(2, 2), &wsa_data);
	if (wsa_res || LOBYTE(wsa_data.wVersion) != 2 ||
	    HIBYTE(wsa_data.wVersion) != 2) {
		error("Network socket setup failed, couldn't start server");
		exit(SERV_ERR_SOCK);
	}
	#endif

	/* `sockopt` boolean definitions, must be `int`s, passed as `char*` */
	int so_true_int = 1;
	char* so_true = (char*) &so_true_int;
	int so_false_int = 0;
	char* so_false = (char*) &so_false_int;
	int so_size = sizeof(int);

	struct sockaddr_in6 listen_addr = {0};
	/* Set the address to ::1 (localhost) */
	memset(&listen_addr.sin6_addr, 0, sizeof(listen_addr.sin6_addr));
	((uint16_t*) (&listen_addr.sin6_addr))[7] = htons(1);
	/* Set the port to the one passed to `create_socket` */
	listen_addr.sin6_port = htons(listen_port);

	int32_t addr_size = sizeof(listen_addr);
	struct sockaddr* addr = (struct sockaddr*) &listen_addr;
	addr->sa_family = AF_INET6;

	Socket sock = socket(AF_INET6, SOCK_STREAM, IPPROTO_TCP);

	#ifdef _WIN32
	if (sock == INVALID_SOCKET) {
		error("Could not open network socket");
		exit(SERV_ERR_SOCK);
	}
	#else
	if (sock < 0) {
		error("Could not open network socket");
		exit(SERV_ERR_SOCK);
	}
	#endif

	/* `SO_REUSEADDR` has different meanings across platforms:
	 * - On Windows, it allows multiple listeners per socket (which is very bad)
	 * - On Unix-like OSs, it allows a process to bind to a recently-closed
	 *   socket (which can occasionally speed up socket initialization)
	 */
	#ifdef _WIN32
	int32_t res = setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, so_false, so_size);
	#else
	int32_t res = setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, so_true, so_size);
	#endif
	if (res || setsockopt(sock, IPPROTO_IPV6, IPV6_V6ONLY, so_false, so_size) ||
	    setsockopt(sock, IPPROTO_TCP, TCP_NODELAY, so_true, so_size) ||
	    bind(sock, addr, addr_size) || listen(sock, SOMAXCONN)) {
		error("Could not open network socket");
		close_socket(sock);
		exit(SERV_ERR_SOCK);
	}

	return sock;
}

bool accept_connection(Socket sock, Socket* incoming) {
	struct sockaddr_in6 addr;
	socklen_t addr_size = sizeof(addr);
	Socket res = accept(sock, (struct sockaddr*) &addr, &addr_size);
	#ifdef _WIN32
	if (res == INVALID_SOCKET || addr_size > sizeof(addr)) {
		return false;
	}
	#else
	if (res < 0) {
		return false;
	}
	#endif
	*incoming = res;

	char trace_buf[103];
	sprintf(trace_buf, "Accepted a new connection from "
	                   "[%04x:%04x:%04x:%04x:%04x:%04x:%04x:%04x]:%d (socket "
					   #ifdef WIN32
	                   "%llu)"
					   #else
					   "%lu)"
					   #endif
					   , ntohs(((uint16_t*) (&addr.sin6_addr))[0]),
	        ntohs(((uint16_t*) (&addr.sin6_addr))[1]),
	        ntohs(((uint16_t*) (&addr.sin6_addr))[2]),
	        ntohs(((uint16_t*) (&addr.sin6_addr))[3]),
	        ntohs(((uint16_t*) (&addr.sin6_addr))[4]),
	        ntohs(((uint16_t*) (&addr.sin6_addr))[5]),
	        ntohs(((uint16_t*) (&addr.sin6_addr))[6]),
	        ntohs(((uint16_t*) (&addr.sin6_addr))[7]), ntohs(addr.sin6_port),
	        (uint64_t) res);
	debug(trace_buf);

	return true;
}

struct Buffer new_buffer(size_t cap) {
	if (cap == 0) {
		cap = SERV_DEFAULT_BUFFER_CAP;
	}

	struct Buffer buf;
	buf.len = 0;
	buf.cap = cap;
	buf.buf = malloc(cap);

	return buf;
}

void free_buffer(struct Buffer buf) {
	free(buf.buf);
}

char* buffer_to_str(struct Buffer buf) {
	uint8_t* new_buf = realloc(buf.buf, buf.len + 1);

	if (new_buf == NULL) {
		buf.buf[min(buf.len, buf.cap - 1)] = 0;
		return (char*) buf.buf;
	} else {
		new_buf[buf.len] = 0;
		return (char*) new_buf;
	}
}

bool receive(Socket sock, struct Buffer* buf) {
	int32_t res = recv(sock, (char*) buf->buf, (int) buf->cap, 0);

	if (res == -1) {
		buf->len = 0;
		return false;
	}

	char trace_buf[61];
	sprintf(trace_buf, "Received %d bytes on socket "
	#ifdef WIN32
	"%llu"
	#else
	"%lu"
	#endif
	, res, (uint64_t) sock);
	trace(trace_buf);

	buf->len = res;
	return true;
}
