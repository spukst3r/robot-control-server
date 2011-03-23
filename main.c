#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <getopt.h>

#include "server.h"
#include "log.h"

struct parameters params;

void parse_cmdline(int argc, char *argv[], struct parameters *params)
{
	struct option long_options[] = {
		{ "help",		0,	NULL,	'h' },
		{ "version",	0,	NULL,	'V' },
		{ "verbose",	1,	NULL,	'v' },
		{ "daemonize",	1,	NULL,	'b' },
		{ "log-file",	1,	NULL,	'l' },
		{ 0, 0, 0, 0 },
	};

	int val, opt_index = 0;

	while ((val = getopt_long(argc, argv,
					"hVv:bl:",
					long_options, &opt_index)) != -1) {
		switch (val) {
			case 'h':
				show_help(argv[0]);
				exit(0);
			case 'V':
				show_version();
				exit(0);
			case 'v':
				if (strlen(optarg) == 1
						&& '0' <= *optarg
						&& '5' >= *optarg) {
					params->verb_level = *optarg - '0';
				} else {
					fprintf(stderr, "Verbosity level should be 0..5!\n");
					exit(-1);
				}
				break;
			case 'b':
				params->daemonize = 1;
				break;
			case 'l':
				if (optarg) {
					free(params->log_file_path);
					params->log_file_path = malloc(strlen(optarg) + 1);
					strcpy(params->log_file_path, optarg);
				}
				break;
			default:
				show_help(argv[0]);
				exit(-1);
		}
	}

	if (*(argv + optind)) {
		fprintf(stderr, "Invalid argument: %s\n",
				*(argv + optind));
	}
}

void default_params(struct parameters *p)
{
	char *path = "/var/log/robo_server.log";
	p->log_file_path = malloc(strlen(path) + 1);

	strcpy(p->log_file_path, path);
}

void show_help(const char *path)
{
	printf("Usage:\n"
			"%s bla bla\n", path);
}

void show_version()
{
	printf("%s\n", SERVER_VERSION);
}

void handle_sigint(int sig)
{
	logit("Caught SIGINT, quitting...\n");
	close(params.log_file);
	free(params.log_file_path);

	exit(0);
}

int main(int argc, char *argv[])
{
	default_params(&params);
	parse_cmdline(argc, argv, &params);

	params.log_file = open(params.log_file_path, O_CREAT | O_TRUNC | O_RDWR, 0640);

	if (params.log_file < 0) {
		perror("open()");
		params.log_file = 1; //stdout
	}

	struct sigaction sigact;

	sigemptyset(&sigact.sa_mask);
	sigact.sa_handler = handle_sigint;
	sigaction(SIGINT, &sigact, 0);

	logit(L_DEBUG "Starting server");

	start_server(&params);
	return 0;
}

