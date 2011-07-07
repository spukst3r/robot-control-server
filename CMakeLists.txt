set (PROJECT "server")

project(${PROJECT})

set (CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall -Werror")
set (SOURCES main.c server.c daemonize.c log.c)

add_executable(server ${SOURCES})

target_link_libraries(${PROJECT} rt)

