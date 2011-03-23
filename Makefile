ARGS=-Wall -Werror

all: server

server: server.o main.o log.o daemonize.o
	gcc ${ARGS} daemonize.o log.o server.o main.o -o server

server.o: server.c server.h
	gcc ${ARGS} -c server.c

log.o: server.h log.h log.c
	gcc ${ARGS} -c log.c

daemonize.o: daemonize.h daemonize.c
	gcc ${ARGS} -c daemonize.c

main.o: main.c server.h
	gcc ${ARGS} -c main.c

clean:
	rm server *.o
