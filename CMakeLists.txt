cmake_minimum_required(VERSION 3.23)
project(c_http_server C)

set(CMAKE_C_STANDARD 90)

add_executable(c_http_server http.c log.c server.c socket.c handlers.c)

if (WIN32)
    target_link_libraries(c_http_server wsock32 ws2_32)
endif ()
