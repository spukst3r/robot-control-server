#ifndef _CONFIG_H_
#define _CONFIG_H_

#include "server.h"

int parse_config(const char *file_name, struct parameters *params);
int parse_line(const char *line, char **key, char **value);
int parse_keyvalue(const char *key, const char *value,
		struct parameters *params);
int check_config(struct parameters *params);

char *trim_string(const char *str);


#endif

