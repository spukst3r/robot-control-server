#include <sys/stat.h>
#include <semaphore.h>
#include <fcntl.h>
#include <unistd.h>

#include <stdarg.h>
#include <string.h>
#include <stdio.h>
#include <time.h>

#include "server.h"
#include "log.h"

extern struct parameters params;

static sem_t *sem;

int logit(const char *format, ...)
{
	va_list ap;
	char buf[512], buf2[512];
	time_t t = time(NULL);
	struct tm *tms = localtime(&t);
	char *levels[] = { L_FATAL, L_ERROR, L_WARNING, L_INFO, L_DEBUG };
	int i, res;

	if (sem == NULL) {
		fprintf(stderr, "call log_init() before using logit()\n");
		return -1;
	}

	sem_wait(sem);

	for (i = 0; i < 5; i++)
		if (strstr(format, levels[i]))
			break;

	if (i >= params.verb_level && i != 5)
		return 0;

	sprintf(buf, "[%02d:%02d:%02d] %s\n", tms->tm_hour, tms->tm_min, tms->tm_sec, format);

	va_start(ap, format);
	vsprintf(buf2, buf, ap);
	va_end(ap);

	res = write(params.log_file, buf2, strlen(buf2));

	sem_post(sem);

	return res;
}

int log_init()
{
	sem = sem_open("/logitsemaphore", O_CREAT | O_EXCL, 660, 1);
	if (sem == SEM_FAILED) {
		perror("sem_open");
		return -1;
	}
	return 0;
}

int log_dispose()
{
	sem_close(sem);
	if (sem_unlink("/logitsemaphore") < 0) {
		perror("sem_unlink");
		return -1;
	}
	return 0;
}

