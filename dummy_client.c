#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include <netdb.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <netinet/in.h>

int main(int argc, char *argv[])
{
	int sock;
	struct sockaddr_in sin = {0};
	struct hostent *host;
	char buf[128];

	if (argc != 2) {
		fprintf(stderr, "Usage: %s server\n", argv[0]);
		exit(-1);
	}

	sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

	if (sock < 0) {
		perror("socket");
		exit(-1);
	}

	host = gethostbyname(argv[1]);

	if (!host) {
		perror("gethostbyname");
		exit(-1);
	}

	sin.sin_family = AF_INET;
	sin.sin_port = htons(1190);
	sin.sin_addr = *((struct in_addr*)host->h_addr);

	if (connect(sock, (struct sockaddr*)&sin, sizeof sin) < 0) {
		perror("connect");
		exit(-1);
	}

	while (1) {
		printf("> ");
		fgets(buf, 128, stdin);
		buf[strlen(buf) - 1] = 0;
		if (send(sock, buf, strlen(buf) + 1, 0) < 0) {
			perror("send");
			exit(-1);
		}
		if (strstr(buf, "exit") == buf)
			break;
	}
	shutdown(sock, 2);
	close(sock);
	return 0;
}

