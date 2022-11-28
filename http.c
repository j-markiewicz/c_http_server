/* Implementation of `http.h`, see that file for documentation and types */

#include "http.h"

#include <stdbool.h>
#include <string.h>
#include <stdlib.h>

#include "socket.h"
#include "log.h"
#include "handlers.h"

enum Method method_from_str(char* str) {
	if (strcmp(str, "GET") == 0) {
		return Get;
	} else if (strcmp(str, "HEAD") == 0) {
		return Head;
	} else if (strcmp(str, "POST") == 0) {
		return Post;
	} else if (strcmp(str, "PUT") == 0) {
		return Put;
	} else if (strcmp(str, "DELETE") == 0) {
		return Delete;
	} else if (strcmp(str, "PATCH") == 0) {
		return Patch;
	} else {
		return Other;
	}
}

struct Path parse_path(const char* path_str) {
	struct Path path = {0};

	/* Special handling of the path "/" to make the rest simpler */
	if (strcmp(path_str, "/") == 0) {
		path.components = malloc(sizeof(char*));
		path.components[0] = malloc(1);
		strcpy(path.components[0], "");

		path.num_components = 1;

		path.query = malloc(1);
		strcpy(path.query, "");

		return path;
	}

	/* Count the number of segments (≤ number of "/" before "?") */
	size_t i = 0;
	while (path_str[i] != '\0' && path_str[i] != '?') {
		if (path_str[i] == '/') {
			path.num_components++;
		}

		i++;
	}

	path.components = malloc(path.num_components * sizeof(char*));

	/* Copy the segments into `path.segments` */
	for (i = 0; i < path.num_components; i++) {
		/* Ignore leading "/" */
		path_str++;

		size_t len = strcspn(path_str, "/?");

		/* Ignore this segment if it's empty (e.g. when the path ends in "/") */
		if (len == 0) {
			i--;
			path.num_components--;
			continue;
		}

		path.components[i] = malloc(len + 1);
		strncpy(path.components[i], path_str, len);
		path.components[i][len] = 0;

		path_str += len;
	}

	/* At this point (assuming the path is well-formed), `path_str` points to
	 * either a null terminator (if the path doesn't have a query string), or
	 * to a "?" (if there is a query string).
	 */
	if (path_str[0] == '\0') {
		path.query = malloc(1);
		strcpy(path.query, "");

		return path;
	}

	/* Skip the "?" */
	path_str++;

	/* Copy the query string */
	size_t query_len = strlen(path_str);
	path.query = malloc(query_len + 1);
	strcpy(path.query, path_str);

	return path;
}

void free_path(struct Path path) {
	if (path.components != NULL) {
		size_t i;
		for (i = 0; i < path.num_components; i++) {
			if (path.components[i] != NULL) {
				free(path.components[i]);
			}
		}

		free(path.components);
	}

	if (path.query != NULL) {
		free(path.query);
	}
}

bool parse_request(const char* text_req, struct Request* req) {
	/* Parse HTTP request method */
	size_t method_len = strcspn(text_req, " ");
	char* method_str = malloc(method_len + 1);
	strncpy(method_str, text_req, method_len);
	method_str[method_len] = 0;

	if ((req->method = method_from_str(method_str)) == Other) {
		free(method_str);
		return false;
	}

	free(method_str);

	/* Parse the HTTP path */
	text_req += method_len + 1;
	size_t path_len = strcspn(text_req, " ");
	char* path_str = malloc(path_len + 1);
	strncpy(path_str, text_req, path_len);
	path_str[path_len] = 0;
	req->path = parse_path(path_str);

	free(path_str);

	return true;
}

bool handle_request(struct Request* req, Socket sock, char* data_dir) {
	uint16_t status;

	switch (req->method) {
		case Get:
			status = handle_get(req->path, sock, data_dir);
			break;
		case Head:
		case Post:
		case Put:
		case Delete:
		case Patch:
		default:
			status = send_501(sock);
			break;
	}

	/* If the status indicates success or a client error */
	if (status >= 200 && status < 500) {
		return true;
	} else {
		return false;
	}
}

char* guess_mime_type(const char* file_ext) {
	/* Below code generated based on data from
	 * https://developer.mozilla.org/en-US/docs/Web/HTTP/Basics_of_HTTP/MIME_types/Common_types
	 */
	if (file_ext == NULL) {
		return "application/octet-stream";
	} else if (strcmp(file_ext, ".aac") == 0) {
		return "audio/aac";
	} else if (strcmp(file_ext, ".abw") == 0) {
		return "application/x-abiword";
	} else if (strcmp(file_ext, ".arc") == 0) {
		return "application/x-freearc";
	} else if (strcmp(file_ext, ".avif") == 0) {
		return "image/avif";
	} else if (strcmp(file_ext, ".avi") == 0) {
		return "video/x-msvideo";
	} else if (strcmp(file_ext, ".azw") == 0) {
		return "application/vnd.amazon.ebook";
	} else if (strcmp(file_ext, ".bmp") == 0) {
		return "image/bmp";
	} else if (strcmp(file_ext, ".bz") == 0) {
		return "application/x-bzip";
	} else if (strcmp(file_ext, ".bz2") == 0) {
		return "application/x-bzip2";
	} else if (strcmp(file_ext, ".cda") == 0) {
		return "application/x-cdf";
	} else if (strcmp(file_ext, ".csh") == 0) {
		return "application/x-csh";
	} else if (strcmp(file_ext, ".css") == 0) {
		return "text/css";
	} else if (strcmp(file_ext, ".csv") == 0) {
		return "text/csv";
	} else if (strcmp(file_ext, ".doc") == 0) {
		return "application/msword";
	} else if (strcmp(file_ext, ".docx") == 0) {
		return "application/vnd.openxmlformats-officedocument.wordprocessingml.document";
	} else if (strcmp(file_ext, ".eot") == 0) {
		return "application/vnd.ms-fontobject";
	} else if (strcmp(file_ext, ".epub") == 0) {
		return "application/epub+zip";
	} else if (strcmp(file_ext, ".gz") == 0) {
		return "application/gzip";
	} else if (strcmp(file_ext, ".gif") == 0) {
		return "image/gif";
	} else if (strcmp(file_ext, ".htm") == 0 || strcmp(file_ext, ".html") == 0) {
		return "text/html";
	} else if (strcmp(file_ext, ".ico") == 0) {
		return "image/vnd.microsoft.icon";
	} else if (strcmp(file_ext, ".ics") == 0) {
		return "text/calendar";
	} else if (strcmp(file_ext, ".jar") == 0) {
		return "application/java-archive";
	} else if (strcmp(file_ext, ".jpeg") == 0 || strcmp(file_ext, ".jpg") == 0) {
		return "image/jpeg";
	} else if (strcmp(file_ext, ".js") == 0 || strcmp(file_ext, ".mjs") == 0) {
		return "text/javascript";
	} else if (strcmp(file_ext, ".json") == 0) {
		return "application/json";
	} else if (strcmp(file_ext, ".jsonld") == 0) {
		return "application/ld+json";
	} else if (strcmp(file_ext, ".mid") == 0 || strcmp(file_ext, ".midi") == 0) {
		return "audio/midi";
	} else if (strcmp(file_ext, ".mp3") == 0) {
		return "audio/mpeg";
	} else if (strcmp(file_ext, ".mp4") == 0) {
		return "video/mp4";
	} else if (strcmp(file_ext, ".mpeg") == 0) {
		return "video/mpeg";
	} else if (strcmp(file_ext, ".mpkg") == 0) {
		return "application/vnd.apple.installer+xml";
	} else if (strcmp(file_ext, ".odp") == 0) {
		return "application/vnd.oasis.opendocument.presentation";
	} else if (strcmp(file_ext, ".ods") == 0) {
		return "application/vnd.oasis.opendocument.spreadsheet";
	} else if (strcmp(file_ext, ".odt") == 0) {
		return "application/vnd.oasis.opendocument.text";
	} else if (strcmp(file_ext, ".oga") == 0) {
		return "audio/ogg";
	} else if (strcmp(file_ext, ".ogv") == 0) {
		return "video/ogg";
	} else if (strcmp(file_ext, ".ogx") == 0) {
		return "application/ogg";
	} else if (strcmp(file_ext, ".opus") == 0) {
		return "audio/opus";
	} else if (strcmp(file_ext, ".otf") == 0) {
		return "font/otf";
	} else if (strcmp(file_ext, ".png") == 0) {
		return "image/png";
	} else if (strcmp(file_ext, ".pdf") == 0) {
		return "application/pdf";
	} else if (strcmp(file_ext, ".php") == 0) {
		return "application/x-httpd-php";
	} else if (strcmp(file_ext, ".ppt") == 0) {
		return "application/vnd.ms-powerpoint";
	} else if (strcmp(file_ext, ".pptx") == 0) {
		return "application/vnd.openxmlformats-officedocument.presentationml.presentation";
	} else if (strcmp(file_ext, ".rar") == 0) {
		return "application/vnd.rar";
	} else if (strcmp(file_ext, ".rtf") == 0) {
		return "application/rtf";
	} else if (strcmp(file_ext, ".sh") == 0) {
		return "application/x-sh";
	} else if (strcmp(file_ext, ".svg") == 0) {
		return "image/svg+xml";
	} else if (strcmp(file_ext, ".tar") == 0) {
		return "application/x-tar";
	} else if (strcmp(file_ext, ".tif") == 0 || strcmp(file_ext, ".tiff") == 0) {
		return "image/tiff";
	} else if (strcmp(file_ext, ".ts") == 0) {
		return "video/mp2t";
	} else if (strcmp(file_ext, ".ttf") == 0) {
		return "font/ttf";
	} else if (strcmp(file_ext, ".txt") == 0) {
		return "text/plain";
	} else if (strcmp(file_ext, ".vsd") == 0) {
		return "application/vnd.visio";
	} else if (strcmp(file_ext, ".wav") == 0) {
		return "audio/wav";
	} else if (strcmp(file_ext, ".weba") == 0) {
		return "audio/webm";
	} else if (strcmp(file_ext, ".webm") == 0) {
		return "video/webm";
	} else if (strcmp(file_ext, ".webp") == 0) {
		return "image/webp";
	} else if (strcmp(file_ext, ".woff") == 0) {
		return "font/woff";
	} else if (strcmp(file_ext, ".woff2") == 0) {
		return "font/woff2";
	} else if (strcmp(file_ext, ".xhtml") == 0) {
		return "application/xhtml+xml";
	} else if (strcmp(file_ext, ".xls") == 0) {
		return "application/vnd.ms-excel";
	} else if (strcmp(file_ext, ".xlsx") == 0) {
		return "application/vnd.openxmlformats-officedocument.spreadsheetml.sheet";
	} else if (strcmp(file_ext, ".xml") == 0) {
		return "application/xml";
	} else if (strcmp(file_ext, ".xul") == 0) {
		return "application/vnd.mozilla.xul+xml";
	} else if (strcmp(file_ext, ".zip") == 0) {
		return "application/zip";
	} else if (strcmp(file_ext, ".3gp") == 0) {
		return "video/3gpp";
	} else if (strcmp(file_ext, ".3g2") == 0) {
		return "video/3gpp2";
	} else if (strcmp(file_ext, ".7z") == 0) {
		return "application/x-7z-compressed";
	} else {
		return "application/octet-stream";
	}
}
