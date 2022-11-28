/* Implementation of `handlers.h`, see that file for documentation and types */

#include "handlers.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>

#ifndef _WIN32
#include <sys/sendfile.h>
#endif

#include "log.h"
#include "http.h"

uint16_t handle_get(struct Path path, Socket sock, char* data_dir) {
	/* Make file path */
	size_t path_len = 1;
	size_t i;
	for (i = 0; i < path.num_components; i++) {
		path_len += strlen(path.components[i]);
	}

	size_t data_dir_len = strlen(data_dir);
	char* file_path = malloc(data_dir_len + path_len + path.num_components + 11);
	strcpy(file_path, data_dir);
	char* file_path_cursor = file_path + data_dir_len;
	for (i = 0; i < path.num_components; i++) {
		size_t component_len = strlen(path.components[i]);
		memcpy(file_path_cursor, path.components[i], component_len);
		file_path_cursor += component_len;
		(*file_path_cursor) = '/';
		file_path_cursor++;
	}
	file_path_cursor[-1] = '\0';

	/* If the path is a directory, try `[path]/index.html` */
	DIR* dir = opendir(file_path);
	if (dir) {
		closedir(dir);
		if (file_path[strlen(file_path) - 1] == '/') {
			strcat(file_path, "index.html");
		} else {
			strcat(file_path, "/index.html");
		}
	}

	/* Read file */
	FILE* file = fopen(file_path, "rb");
	if (file == NULL) {
		warn("The file could not be opened");
		return send_404(sock);
	}

	if (fseek(file, 0, SEEK_END)) {
		error("Can't get file size");
		return send_500(sock);
	}
	size_t file_size = ftell(file);
	if (file_size == -1) {
		error("Can't get file size");
		return send_500(sock);
	}
	rewind(file);

	/* Guess MIME type from file extension */
	char* path_ext = strrchr(file_path, '.');
	char* mime_type = guess_mime_type(path_ext);

	/* Send status and headers */
	char* buf = malloc(74 + strlen(mime_type));
	sprintf(buf, "HTTP/1.1 200 OK\r\nContent-Length: "
	#ifdef WIN32
	"%llu"
	#else
	"%lu"
	#endif
	"\r\nContent-Type: %s\r\n\r\n", (uint64_t) file_size, mime_type);
	size_t buf_len = strlen(buf);

	int32_t res = send(sock, buf, (int) buf_len, 0);
	if (res == -1) {
		error("Error sending response data");
	} else if (res != buf_len) {
		warn("Couldn't send full response");
	}

	free(buf);

	/* Read and send file */
	uint8_t* content = malloc(file_size);
	size_t content_size = fread(content, 1, file_size, file);
	if (content_size != file_size) {
		warn("Couldn't read entire file");
	}

	fclose(file);
	free(file_path);

	uint8_t* content_cursor = content;
	while (content_size > 0) {
		size_t amount = send(sock, (char*) content_cursor, (int) content_size,
		                     0);
		if (amount == -1) {
			warn("Couldn't send data");
			break;
		} else {
			content_size -= amount;
			content_cursor += amount;
		}
	}

	free(content);

	return content_size == 0 ? 200 : 0;
}

uint16_t send_404(Socket sock) {
	char* buf = "HTTP/1.1 404 Not Found\r\n\r\n";
	size_t buf_len = strlen(buf);

	int32_t res = send(sock, buf, (int) buf_len, 0);
	if (res == -1) {
		error("Error sending response data");
	} else if (res != buf_len) {
		warn("Couldn't send full response");
	}

	return 404;
}

uint16_t send_500(Socket sock) {
	char* buf = "HTTP/1.1 500 Internal Server Error\r\n\r\n";
	size_t buf_len = strlen(buf);

	int32_t res = send(sock, buf, (int) buf_len, 0);
	if (res == -1) {
		error("Error sending response data");
	} else if (res != buf_len) {
		warn("Couldn't send full response");
	}

	return 500;
}

uint16_t send_501(Socket sock) {
	char* buf = "HTTP/1.1 501 Not Implemented\r\n\r\n";
	size_t buf_len = strlen(buf);

	int32_t res = send(sock, buf, (int) buf_len, 0);
	if (res == -1) {
		error("Error sending response data");
	} else if (res != buf_len) {
		warn("Couldn't send full response");
	}

	return 501;
}
