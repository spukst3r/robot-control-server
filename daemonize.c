#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#include "daemonize.h"

int daemonize()
{
	int i, pid;

	switch (fork()) {
		case -1:
			perror("fork()");
			exit(-1);
		case 0:
			if ((pid = setsid()) < 0) {
				perror("setsid()");
				exit(-1);
			}

			//for (i = getdtablesize(); i>=0; --i)
				//close(i);

			i = open("/dev/null", O_RDWR);
			//dup2(i, STDIN_FILENO);
			//dup2(i, STDOUT_FILENO);

			umask(027);

			chdir("/");

			return pid;

		default:
			exit(0);
	}
}

