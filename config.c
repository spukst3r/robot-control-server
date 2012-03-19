#include "config.h"
#include "log.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <ctype.h>

static const char *keys[] = {
	"server_port", "server_max_clients", "server_verbosity",
	"server_daemonize", "server_log_file", NULL
};

int parse_config(const char *file_name, struct parameters *params)
{
	FILE *f;
	char buf[512], *key, *value;
	unsigned int line_number = 0;

	logit(L_INFO "Opening config file: %s", file_name);

	if ((f = fopen(file_name, "rb")) == NULL) {
		logit(L_ERROR "Error opening config file: %s", strerror(errno));
		return -1;
	}

	while (fgets(buf, sizeof buf, f)) {
		++line_number;

		if (parse_line(buf, &key, &value)) {
			logit(L_FATAL "Error parsing config file on line %d: %s",
					line_number, buf);
			fclose(f);
			return -1;
		}

		if (key != NULL) {
			if (parse_keyvalue(key, value, params)) {
				logit(L_FATAL "Error parsing config file: line %d: %s",
						line_number, buf);
				return -1;
			}
		}

		free(key);
		free(value);
	}

	fclose(f);
	return 0;
}

int parse_line(const char *line, char **key, char **value)
{
	char *sep, *trimmed;

	trimmed = trim_string(line);

	if (!*trimmed || *trimmed == '#') {
		*key = *value = NULL;
		return 0;
	}

	if (!(sep = strchr(trimmed, '=')))
		return -1;

	*sep++ = '\0';

	*key = trim_string(trimmed);
	*value = trim_string(sep);

	free(trimmed);

	return 0;
}

char *trim_string(const char *str)
{
	size_t len = strlen(str);
	const char *end = str + len - 1;
	char *stripped;
	unsigned int from_end = 0, new_size;

	while (isspace(*str) && *str)
		++str;

	len = strlen(str);

	while (isspace(*end) && end > str) {
		++from_end;
		--end;
	}

	new_size = len - from_end;

	stripped = malloc(new_size + 1);
	if (stripped) {
		strncpy(stripped, str, new_size);
		stripped[new_size] = '\0';
	}

	return stripped;
}

int parse_keyvalue(const char *key, const char *value,
		struct parameters *params)
{
	const char **current_key;
	unsigned int cmd = 0;
	int temp;

	for (current_key=keys; *current_key; ++current_key, ++cmd) {
		if (!strcmp(*current_key, key))
			break;
	}

	if (!*current_key)
		return -1;

	logit(L_DEBUG "cmd index: %d", cmd);
	switch (cmd) {
		case 0: /* server_port */
			temp = atoi(value);
			if (temp <= 0 || temp >= 65535)
				return -1;

			logit(L_DEBUG "parse_config: %s = %d", key, temp);
			params->server_port = temp;
			break;

		case 1: /* max_clients */
			break;
	}

	return 0;
}

