/* The non-cmake entrypoint to this server program. This file exists to make
 * this project compile with just `gcc main.c`. It is not used when building
 * with CMake.
 */

#include "log.c"
#include "socket.c"
#include "http.c"
#include "handlers.c"
#include "server.c"
