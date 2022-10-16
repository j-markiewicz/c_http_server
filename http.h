/* A very basic HTTP/1.1[1]-compatible-ish parser for use in this server only
 * and never in any production environment
 *
 * [1] Fielding, R., Ed., Nottingham, M., Ed., and J. Reschke, Ed., "HTTP/1.1",
 * STD 99, RFC 9112, June 2022, <https://www.rfc-editor.org/info/std99>.
 */

#ifndef C_HTTP_SERVER_HTTP_H
#define C_HTTP_SERVER_HTTP_H

#include <stdbool.h>
#include <stdint.h>

#include "socket.h"

/* A supported HTTP request method */
enum Method {
	Get,
	Head,
	Post,
	Put,
	Delete,
	Patch,
	Other
};

/* Parse an HTTP method from the given string. Returns `Other` if the method is
 * not known or unsupported.
 */
enum Method method_from_str(char* str);

/* The path of an HTTP request */
struct Path {
	/* Individual path components, originally separated by "/" */
	char** components;
	/* The number of path components */
	size_t num_components;
	/* The full query string, without "?" */
	char* query;
};

/* Parse a Path from a string containing its origin-form (see
 * <https://www.rfc-editor.org/rfc/rfc9112#name-origin-form>). This function
 * allocates memory for the Path, which should be freed using `free_path`.
 */
struct Path parse_path(const char* path_str);

/* Free the given path. If any pointers within `path` are NULL, they are
 * ignored, so passing a 0-initialized Path to this function is safe.
 */
void free_path(struct Path path);

/* An HTTP request */
struct Request {
	enum Method method;
	struct Path path;
};

/* Parse an HTTP request from the provided buffer into the request struct
 * pointed to by `req`. Returns true if parsing was successful, false
 * otherwise. If this function fails, `req` may have been partially modified.
 */
bool parse_request(const char* text_req, struct Request* req);

/* Handle an HTTP request using the provided request information in `req`.
 * Returns true if the request was handled without server error (HTTP status
 * code 2XX/3XX/4XX, and no fatal errors in the handlers).
 */
bool handle_request(struct Request* req, Socket sock, char* data_dir);

/* Guess the mime type by the provided file extension (with '.') */
char* guess_mime_type(const char* file_ext);

#endif
