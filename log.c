#include <sys/stat.h>
#include <sys/time.h>
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

static sem_t sem;
static int called_init;

int logit(const char *format, ...)
{
	va_list ap;
	char buf[512], buf2[512];
	time_t t = time(NULL);
	struct tm *tms = localtime(&t);
	struct timeval tval;
	struct timezone tzone;
	char *levels[] = { L_FATAL, L_ERROR, L_WARNING, L_INFO, L_DEBUG };
	int i, res;

	if (!called_init) {
		fprintf(stderr, "Call log_init first!\n");
	}

	for (i = 0; i < 5; i++)
		if (strstr(format, levels[i]))
			break;

	if (i >= params.verb_level && i != 5)
		return 0;

	sem_wait(&sem);

	gettimeofday(&tval, &tzone);
	sprintf(buf, "[%04d.%02d.%02d %02d:%02d:%02d.%06ld] %s\n",
			tms->tm_year + 1900, tms->tm_mon + 1, tms->tm_mday,
			tms->tm_hour, tms->tm_min, tms->tm_sec,
			tval.tv_usec, format);

	va_start(ap, format);
	vsprintf(buf2, buf, ap);
	va_end(ap);

	res = write(params.log_file, buf2, strlen(buf2));

	sem_post(&sem);

	return res;
}

// TODO: move all settings from params to log_init
int log_init()
{
	int ret = sem_init(&sem, 1, 1);

	if (ret < 0) {
		perror("sem_open");
		return -1;
	}
	called_init = 1;
	return 0;
}

int log_dispose()
{
	called_init = 0;
	if (sem_destroy(&sem) < 0) {
		perror("sem_destroy");
		return -1;
	}
	return 0;
}

