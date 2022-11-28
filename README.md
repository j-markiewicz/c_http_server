# C HTTP Server

## Compiling

The `main.c` file is provided to aid with compilation. Simply using `gcc main.c -o server` will compile this project
into a binary named `server`. From there, it can be run with `./server -d test-data -p 51234` to listen on port `51234`,
and serve files from `./test-data/`.

You can also use CMake with the provided `CMakeLists.txt` file, or compile using
`gcc -ansi -o server log.c socket.c http.c handlers.c server.c`.

On Windows, during compilation `winsock2` also needs to be linked.

## Demo (Linux with GCC)

1. Compile the server by running `gcc -o server main.c` in the directory that this file is in.
2. Start the server on port 8000 using `./server -p 8000 -d test-data`.
3. Using a web browser, navigate to `http://localhost:8000`. The response will contain the `test-data/index.html` file.
4. Navigate to `http://localhost:8000/about.html` to get links to more files to try out.

## Features

The server will respond to a `GET` request with the file at the requested location, relative to the `-d` argument.
Files, that do not exist are correctly handled with a `404` response, while methods other than `GET` get a `501` status.
When sending the file, the server attempts to guess the file's mime type from the file extension.
If the user requests a directory, the server automatically tries sending that directory's `index.html` file.

## File contents

| file name    | content                                                              |
|--------------|----------------------------------------------------------------------|
| `main.c`     | `#include` directives to make compiling easier                       |
| `server.c`   | main server entrypoint, argument parsing, startup logic              |
| `log.c`      | logging helper functions                                             |
| `socket.c`   | cross-platform (Unix and Windows) network sockets                    |
| `http.c`     | HTTP request parsing and helper functions                            |
| `handlers.c` | HTTP request handling, response generation/sending                   |
| `*.h`        | type definitions/function signatures for the corresponding `.c` file |
| `misc.h`     | miscellaneous `#define`s for the entire project                      |

## How a request gets handled

0. On server startup, after parsing the command-line arguments, a TCP socket is opened on the specified port (or 8000)
1. An HTTP request is sent to the listening socket (for example by a browser)
2. The request is read into a growable heap-allocated buffer in `server.c` (`L134 - L141`)
3. The request is parsed in `server.c` (`L143 - L152`) and `http.c`
4. The request is handled in `server.c` (`L154 - L156`), `http.c` (`L147 - L170`), and `handlers.c`
5. In the appropriate `handle_*` or `send_*` function (`handlers.c`) the response is generated and sent
   - For a `GET` request, in `handle_get`, the request path (parsed in step 3) is converted into a file path
   - If that file path is a directory, `index.html` is appended to the end of the path
   - The file is read, its length is calculated, its mime type is guessed from its file extension
   - The HTTP status line, response headers, and body (the file contents) are formatted and sent to the client
6. The connection is closed, the server is ready for more requests

## Goals

- Be relatively simple
- Use ANSI C only (except forbidding mixed declaration and statements)
- Work on Windows 10/11 and Linux (Ubuntu 18.04/22.04)
- Be fully written by me ~~maybe with a bit of help from stackoverflow~~
- Compile with just `gcc main.c`
- No external dependencies (other than the standard library)
- Work with modern web browsers
- Moderate HTTP/1.1 spec-compliance

## Non-goals

- Be fully-featured
- Be fully cross-platform
- Be super performant
- Be safe and/or secure
- Handle malformed or malicious inputs
- Follow best practices regarding HTTP
- Handle most edge cases (even some expected ones)
- Fully implement HTTP (any version)
- Have advanced features
