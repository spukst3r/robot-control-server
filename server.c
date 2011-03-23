#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <fcntl.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>

#include "server.h"
#include "daemonize.h"
#include "log.h"

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
	if (listen(s, 6) < 0) {
		perror("listen");
		return -1;
	}

	return s;
}

int start_server(struct parameters *params)
{
	int i;
	int listen_socket,
		cl_socket,
		clients[MAX_CLIENTS] = {0};
	int exiting = 0;
	struct sockaddr_in client;
	socklen_t addrlen = sizeof client;

	if (params->daemonize) {
		daemonize();
	}

	listen_socket = create_listen_socket(SERVER_PORT);

	if (listen_socket < 0) {
		logit(L_FATAL "Failed to create listen socket");
		exit(-1);
	}

	/* Main loop */
	while (!exiting) {
		if ((cl_socket = accept(listen_socket, (struct sockaddr*)&client, &addrlen)) > 0) {
			switch (vfork()) {
				case 0:
					serve_client(cl_socket);
					break;
				case -1:
					perror("vfork");
					exit(-1);
					break;
				default:
					for (i = 0; i<MAX_CLIENTS; i++) {
						if (clients[i] == 0)
							break;
					}
					if (i == MAX_CLIENTS) {
					}
					break;
			}
		}
	}

	return 0;
}

void serve_client(int sock)
{
	if (send(sock, "Hello!", 7, 0) < 0) {
		perror("send");
		_exit(-1);
	}
	close(sock);
}

