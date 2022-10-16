/* Logging functions and types used in the HTTP server*/

#ifndef C_HTTP_SERVER_LOG_H
#define C_HTTP_SERVER_LOG_H

#include <stdbool.h>
#include <stddef.h>

enum Level {
	Error,
	Warn,
	Info,
	Debug,
	Trace
};

/* Return the corresponding string for the provided log_msg level. The returned
 * string is always 5 characters long (not including the null terminator)
 */
const char* level_to_str(enum Level level);

/* Get the current time formatted as an ISO8601/RFC3339 date + time string in
 * UTC inside the provided buffer. The buffer should be at least 21 bytes long.
 * Returns true if the operation succeeded, false if it failed.
 */
bool rfc3339_timestamp(char* buf, size_t len);

/* Print a null-terminated log_msg message without newline character to the
 * standard output, returning whether the operation was completed successfully
 */
bool log_msg(enum Level level, const char* message);

/* Print an error-level null-terminated log_msg message to stdout */
void error(const char* message);

/* Print a warn-level null-terminated log_msg message to the standard output */
void warn(const char* message);

/* Print an info-level null-terminated log_msg message to the standard output */
void info(const char* message);

/* Print a debug-level null-terminated log_msg message to the standard output */
void debug(const char* message);

/* Print a trace-level null-terminated log_msg message to the standard output */
void trace(const char* message);

#endif
