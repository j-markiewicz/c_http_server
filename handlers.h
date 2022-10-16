/* Per-method HTTP request handlers */

#ifndef C_HTTP_SERVER_HANDLERS_H
#define C_HTTP_SERVER_HANDLERS_H

#include <stdbool.h>
#include <stdint.h>

#include "http.h"
#include "socket.h"

/* Handle a GET request, sending the requested file back as an HTTP response.
 * Returns the HTTP status code.
 */
uint16_t handle_get(struct Path path, Socket sock, char* data_dir);

/* Send a "404 Not Found" response. Returns 404. */
uint16_t send_404(Socket sock);

/* Send a "500 Internal Server Error" response. Returns 500. */
uint16_t send_500(Socket sock);

/* Send a "501 Not Implemented" response. Returns 501. */
uint16_t send_501(Socket sock);

#endif
