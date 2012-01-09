#include <fcntl.h>
#include <unistd.h>
#include <pthread.h>
#include <semaphore.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include <errno.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "server.h"
#include "daemonize.h"
#include "log.h"

extern struct client **clients;
extern int listen_socket;

sem_t thread_start_lock, thread_arr_lock;

int create_listen_socket(int port)
{
	struct sockaddr_in addr = {0};
	int s;

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
	if (listen(s, 5) < 0) {
		perror("listen");
		return -1;
	}

	return s;
}

int start_server(struct parameters *params)
{
	int i;
	int cl_socket, val = 1;
	int exiting = 0,
		ret = 0;
	struct sockaddr_in client;
	socklen_t addrlen = sizeof client;
	pthread_t thread;

	if (sem_init(&thread_start_lock, 1, 0)) {
		logit(L_FATAL "Failed to create thread_start_lock");
		exit(-7);
	}

	if (sem_init(&thread_arr_lock, 1, 1)) {
		logit(L_FATAL "Failed to create thread_arr_lock");
		exit(-8);
	}

	if (params->daemonize) {
		daemonize();
	}

	listen_socket = create_listen_socket(params->server_port);

	if (listen_socket < 0) {
		logit(L_FATAL "Failed to create listen socket");
		exit(-1);
	}

	if (setsockopt(listen_socket, SOL_SOCKET, SO_REUSEADDR, &val, sizeof val) < 0)
		logit(L_WARNING "setsockopt() failed: %s", strerror(errno));

	/* Main loop */
	while (!exiting) {

		if ((cl_socket = accept(listen_socket, (struct sockaddr*)&client,
						&addrlen)) > 0) {
			void *thread_args[] = {
				&cl_socket, &i
			};

			for (i=0; i<params->max_clients; i++) {
				logit(L_DEBUG "i = %d", i);
				if (clients[i] == NULL) break;
			}

			if (i == params->max_clients) {
				logit(L_WARNING "Too many clients, dropping client [%s]",
						inet_ntoa(client.sin_addr));
				send(cl_socket, "err_limit_reached", 18, 0);
				while (close(cl_socket) < 0);
				continue;
			}

			clients[i] = calloc(1, sizeof(struct client));

			if ((ret = pthread_create(&thread, NULL,
							serve_client, (void*)thread_args)) != 0) {
				logit(L_FATAL "failed to create thread, error code: %d", ret);
				exit(-6);
			}

			sem_wait(&thread_start_lock);

			clients[i]->thread_id = thread;
			clients[i]->socket    = cl_socket;
		}
	}

	return 0;
}

void *serve_client(void *data)
{
	int sock = *((int*)((void**)data)[0]);
	int i    = *((int*)((void**)data)[1]);
	int l    = 0;
	char buf[128] = { 0 };

	sem_post(&thread_start_lock);

	while (strcmp(buf, "exit") != 0) {
		if ((l = recv(sock, buf, 128, 0)) < 0)
			break;
		if (l == 0) {
			logit(L_DEBUG "peer disconnected");
			break;
		}

		logit(L_DEBUG "recieved: '%s'", buf);
	}

	shutdown(sock, SHUT_RDWR);
	close(sock);

	sem_wait(&thread_arr_lock);
	free(clients[i]);
	clients[i] = 0;
	sem_post(&thread_arr_lock);

	logit(L_DEBUG "Quitting thread");

	return 0;
}

