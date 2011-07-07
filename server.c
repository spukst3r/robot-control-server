#include <fcntl.h>
#include <unistd.h>
#include <semaphore.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "server.h"
#include "daemonize.h"
#include "log.h"

extern struct client **clients;
extern int listen_socket;
extern sem_t *main_process_sem;

int create_listen_socket(int port)
{
	struct sockaddr_in addr = {0};
	int s;//, sem;

	addr.sin_port        = htons(port);
	addr.sin_addr.s_addr = htonl(INADDR_ANY);
	addr.sin_family      = AF_INET;

	if ((s = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP)) < 0) {
		perror("socket");
		return -1;
	}
	if (bind(s, (struct sockaddr*)&addr, sizeof addr) < 0) {
		perror("bind");
		return -1;
	}
	if (listen(s, 6) < 0) {
		perror("listen");
		return -1;
	}

	return s;
}

int start_server(struct parameters *params)
{
	int i;
	int cl_socket, val = 1;
	int exiting = 0;
	pid_t pid;
	struct sockaddr_in client;
	socklen_t addrlen = sizeof client;

	//if ()

	if (params->daemonize) {
		daemonize();
	}

	listen_socket = create_listen_socket(params->server_port);

	if (listen_socket < 0) {
		logit(L_FATAL "Failed to create listen socket");
		exit(-1);
	}

	if (setsockopt(listen_socket, SOL_SOCKET, SO_REUSEADDR, &val, sizeof val) < 0) {
		perror("setsockopt");
		logit(L_WARNING "setsockopt() failed");
	}

	/* Main loop */
	while (!exiting) {

		if ((cl_socket = accept(listen_socket, (struct sockaddr*)&client,
						&addrlen)) > 0) {
			for (i=0; i<params->max_clients; i++)
				if (clients[i] == 0) break;

			if (i == params->max_clients) {
				logit(L_WARNING "Too many clients, dropping client [%s]",
						inet_ntoa(client.sin_addr));
				continue;
			}

			switch ((pid = fork())) {
				case 0:
					serve_client(cl_socket);
					break;

				case -1:
					perror("fork");
					exit(-1);
					break;

				default:
					logit(L_INFO "Client [%s] connected", inet_ntoa(client.sin_addr));

					clients[i] = calloc(1, sizeof(struct client));
					clients[i]->pid    = pid;
					clients[i]->socket = cl_socket;
					break;
			}
		}
	}

	return 0;
}

void serve_client(int sock)
{
	char buf[128] = { 0 };
	while (strcmp(buf, "exit") != 0) {
		if (recv(sock, buf, 128, 0) < 0)
			break;

		logit(L_DEBUG "recieved: '%s'", buf);
	}
	shutdown(sock, SHUT_RDWR);
	close(sock);

	exit(0);
}

