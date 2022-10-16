/* Miscellaneous defines for the c http server */

#ifndef C_HTTP_SERVER_MISC_H
#define C_HTTP_SERVER_MISC_H

#ifndef min
#define min(a, b) (((a) < (b)) ? (a) : (b))
#endif

#ifndef max
#define max(a, b) (((a) > (b)) ? (a) : (b))
#endif

/* Server command-line help string */
#define CLI_HELP "Simple HTTP server usage:\n\
'-h' to show this message\n\
'-d PATH' to serve files from the (relative) PATH (default '.')\n\
'-p PORT' to specify the port to listen on (default 8000)\n\n\
Press [ENTER] to exit.\n"

/* Miscellaneous unrecoverable errors */
#define SERV_ERR_MISC 1

/* The help string was printed and the server is stopping */
#define SERV_ERR_HELP 2

/* Argument parsing failed */
#define SERV_ERR_ARGS 3

/* A log message could not be written to stdout */
#define SERV_ERR_LOG 4

/* A network socket could not be set up */
#define SERV_ERR_SOCK 5

#endif
