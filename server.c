/* A simple HTTP web server as a University project
 * 
 * To compile and start, run "gcc -o server main.c && ./server -d test-data".
 *
 * This server will serve files from a directory, mapping request URLs to file
 * paths and serving those files.
 */

#include <unistd.h>
#include <getopt.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "misc.h"
#include "log.h"
#include "socket.h"
#include "http.h"

int32_t main(int32_t argc, char** argv) {
	info("Starting HTTP server");

	/* Get command-line arguments */
	char* listen_port_str = NULL;
	char* data_dir_str = NULL;
	int32_t c;

	opterr = 0;
	optarg = 0;

	while ((c = getopt(argc, argv, "hp:d:")) != -1) {
		switch (c) {
			case 'h':
				/* `-h` - Print help string and exit after a keypress */
				puts(CLI_HELP);
				getchar();
				return SERV_ERR_HELP;
			case 'p':
				/* `-p` - Set the listening port */
				listen_port_str = optarg;
				break;
			case 'd':
				/* `-d` - Set the HTTP data directory */
				data_dir_str = optarg;
				break;
			case '?':
				/* Unknown option or no value when required */
				if (optopt == 'p') {
					error("Option -p (port) requires a value");
					return SERV_ERR_ARGS;
				} else if (optopt == 'd') {
					error("Option -d (directory) requires a value");
					return SERV_ERR_ARGS;
				} else {
					char buf[32] = "Unknown command-line option '\0'";
					buf[29] = (char) optopt;
					warn(buf);
					return SERV_ERR_ARGS;
				}
			default:
				return SERV_ERR_ARGS;
		}
	}

	/* Parse command-line arguments */
	uint16_t listen_port = 8000;
	size_t path_max_len = 2048;
	char* data_dir = malloc(path_max_len);
	memset(data_dir, 0, path_max_len);

	if (listen_port_str != NULL) {
		#ifdef WIN32
		uint64_t parsed = strtoull(listen_port_str, NULL, 10);
		#else
		uint64_t parsed = strtoul(listen_port_str, NULL, 10);
		#endif
		if (parsed == 0 || parsed >= (2 << 16)) {
			warn("Invalid listen port (-p) specified");
		} else {
			listen_port = (uint16_t) parsed;
		}
	}

	if (getcwd(data_dir, (int) path_max_len) == NULL) {
		error("Could not get current working directory");
		return SERV_ERR_MISC;
	}

	if (data_dir[strlen(data_dir)] != '/') {
		data_dir[strlen(data_dir)] = '/';
	}

	if (data_dir_str != NULL) {
		if (data_dir_str[0] == '/' || data_dir_str[0] == '~') {
			error("Path (-d) must be relative");
			return SERV_ERR_ARGS;
		}

		strcat(data_dir, data_dir_str);

		if (data_dir[strlen(data_dir)] != '/') {
			data_dir[strlen(data_dir)] = '/';
		}
	}

	char* new_data_dir = realloc(data_dir, strlen(data_dir) + 1);
	if (new_data_dir != NULL) {
		data_dir = new_data_dir;
	}

	char* buf = malloc(26);
	sprintf(buf, "Listening on port '%d'", listen_port);
	info(buf);
	free(buf);

	buf = malloc(20 + strlen(data_dir) + 1);
	sprintf(buf, "Serving data from '%s'", data_dir);
	info(buf);
	free(buf);

	/* Start listening */
	Socket sock = create_socket(listen_port);

	/* Serve HTTP requests */
	while (true) {
		Socket incoming;
		if (!accept_connection(sock, &incoming)) {
			error("Could not accept incoming connection");
			continue;
		}

		struct Buffer buffer = new_buffer(0);
		if (!receive(incoming, &buffer)) {
			error("Could not read any data from connection");
			free_buffer(buffer);
			continue;
		}

		char* http_req = buffer_to_str(buffer);

		struct Request req = {0};
		if (!parse_request(http_req, &req)) {
			error("Could not parse HTTP request");
			free(http_req);
			free_path(req.path);
			close_socket(incoming);
			continue;
		}

		free(http_req);

		if (!handle_request(&req, incoming, data_dir)) {
			error("Could not handle HTTP request");
		}

		free_path(req.path);
		close_socket(incoming);
	}

	close_socket(sock);
	free(data_dir);

	return EXIT_SUCCESS;
}
