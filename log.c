/* Implementation of `log.h`, see that file for documentation and types */

#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <time.h>

#include "log.h"
#include "misc.h"

const char* level_to_str(enum Level level) {
	switch (level) {
		case Trace:
			return "TRACE";
		case Debug:
			return "DEBUG";
		case Info:
			return " INFO";
		case Warn:
			return " WARN";
		case Error:
		default:
			return "ERROR";
	}
}

bool rfc3339_timestamp(char* buf, size_t len) {
	time_t t = time(NULL);
	struct tm* utc_time = gmtime(&t);
	size_t res = strftime(buf, len, "%Y-%m-%dT%H:%M:%SZ", utc_time);
	return res != 0;
}

bool log_msg(enum Level level, const char* message) {
	char timestamp[21] = {0};
	int32_t res;
	if (!rfc3339_timestamp(timestamp, sizeof(timestamp))) {
		return false;
	}

	res = printf("%s - %s: %s\n", level_to_str(level), timestamp, message);
	if (res < 0) {
		return false;
	}

	return true;
}

void error(const char* message) {
	if (!log_msg(Error, message)) {
		exit(SERV_ERR_LOG);
	}
}

void warn(const char* message) {
	if (!log_msg(Warn, message)) {
		exit(SERV_ERR_LOG);
	}
}

void info(const char* message) {
	if (!log_msg(Info, message)) {
		exit(SERV_ERR_LOG);
	}
}

void debug(const char* message) {
	if (!log_msg(Debug, message)) {
		exit(SERV_ERR_LOG);
	}
}

void trace(const char* message) {
	if (!log_msg(Trace, message)) {
		exit(SERV_ERR_LOG);
	}
}
