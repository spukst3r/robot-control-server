#include <stdarg.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <time.h>

#include "server.h"
#include "log.h"

extern struct parameters params;

int logit(const char *format, ...)
{
	va_list ap;
	char buf[512], buf2[512];
	time_t t = time(NULL);
	struct tm *tms = gmtime(&t);
	char *levels[] = { L_FATAL, L_ERROR, L_WARNING, L_INFO, L_DEBUG };
	int i;

	for (i = 0; i < 5; i++)
		if (strstr(format, levels[i]))
			break;

	if (i > params.verb_level && i != 5)
		return 0;

	sprintf(buf, "[%2d:%2d:%2d]: %s\n", tms->tm_hour, tms->tm_min, tms->tm_sec, format);

	va_start(ap, format);
	vsprintf(buf2, buf, ap);
	va_end(ap);

	return write(params.log_file, buf2, strlen(buf2));
}

