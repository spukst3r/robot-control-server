set (PROJECT "server")

project(${PROJECT})

set (SOURCES main.c server.c daemonize.c log.c)

add_executable(server ${SOURCES})

