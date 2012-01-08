set (PROJECT "server")

project(${PROJECT})

set (CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall -pipe")
set (SOURCES main.c server.c daemonize.c log.c)

add_executable(${PROJECT} ${SOURCES})
add_executable(dummy_client dummy_client.c)

target_link_libraries(${PROJECT} rt pthread)

