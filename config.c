#include "config.h"
#include "log.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <ctype.h>

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
	int temp;

	if (!strcmp("server_port", key)) {
		params->server_port = atoi(value);

	} else if (!strcmp("server_max_clients", key)) {
		params->max_clients = atoi(value);

	} else if (!strcmp("server_log_file", key)) {
		free(params->log_file_path);
		params->log_file_path = malloc(strlen(value) + 1);
		strcpy(params->log_file_path, value);

	} else if (!strcmp("server_daemonize", key)) {
		temp = atoi(value);

		if (temp)
			params->daemonize = 1;
		else
			params->daemonize = 0;

	} else if (!strcmp("server_verbosity", key)) {
		params->verb_level = atoi(value);

	} else {
		return -1;
	}

	return 0;
}

int check_config(struct parameters *params)
{
	if (params->server_port < 1 || params->server_port > 65535) {
		fprintf(stderr, "Server port must be in range 1..65535!\n");
		return -1;
	}
	if (params->verb_level < 0 || params->verb_level > 5) {
		fprintf(stderr, "Verbosity level must be in range 0..5!\n");
		return -1;
	}
	if (params->max_clients < 1 || params->max_clients > 10000) {
		fprintf(stderr, "Maximum client number must be in range 1..10000!\n");
		return -1;
	}

	return 0;
}

