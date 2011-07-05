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
struct client **clients;

void parse_cmdline(int argc, char *argv[], struct parameters *params)
{
	struct option long_options[] = {
		{ "help",        0,  NULL,   'h' },
		{ "version",     0,  NULL,   'V' },
		{ "verbosity",   1,  NULL,   'v' },
		{ "daemonize",   1,  NULL,   'b' },
		{ "log-file",    1,  NULL,   'l' },
		{ "port",        1,  NULL,   'p' },
		{ "max-clients", 1,  NULL,   'M' },
		{ 0, 0, 0, 0 },
	};

	int val, opt_index = 0, arg;

	while ((val = getopt_long(argc, argv,
					"hVv:bl:p:M:",
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
				free(params->log_file_path);
				params->log_file_path = malloc(strlen(optarg) + 1);
				strcpy(params->log_file_path, optarg);
				break;

			case 'p':
				arg = atoi(optarg);
				if (arg <= 0 || arg > 65535) {
					fprintf(stderr, "Invalid port number: %d\n", arg);
					exit(-1);
				}
				params->server_port = arg;
				break;

			case 'M':
				arg = atoi(optarg);
				if (arg <=0 || arg > 10000) {
					fprintf(stderr, "Maximum client number out of range 1..10000");
					exit(-1);
				}
				params->max_clients = arg;
				break;

			default:
				show_help(argv[0]);
				exit(-1);
		}
	}

	if (*(argv + optind)) {
		fprintf(stderr, "Invalid argument: %s\n",
				*(argv + optind));
		exit(-2);
	}
}

void default_params(struct parameters *p)
{
	char *path = "/var/log/robo_server.log";
	p->log_file_path = malloc(strlen(path) + 1);

	strcpy(p->log_file_path, path);

	p->verb_level  = 3;
	p->max_clients = 10;
	p->server_port = DEFAULT_SERVER_PORT;
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

void clean_up()
{
	int i;

	logit(L_DEBUG "Cleaning up...");

	close(params.log_file);
	free(params.log_file_path);

	for (i=0; i<params.max_clients; i++) {
		if (clients[i])
			free(clients[i]);
	}

	free(clients);
}

void handle_sigint(int sig)
{
	logit("Caught SIGINT, quitting...\n");

	clean_up();

	exit(0);
}

int main(int argc, char *argv[])
{
	struct sigaction sigact;

	default_params(&params);
	parse_cmdline(argc, argv, &params);

	if (strcmp(params.log_file_path, "-") == 0)
		params.log_file = 2; //stderr
	else
		params.log_file = open(params.log_file_path, O_CREAT | O_TRUNC | O_RDWR, 0640);

	if (params.log_file < 0) {
		perror("open()");
		params.log_file = 1; //stdout
		logit(L_WARNING "Failed to open log file for writing, logging to stdout...");
	}

	logit(L_DEBUG "Command line parameters:\n"
			"\tverb_level:\t%d\n"
			"\tdaemonize:\t%d\n"
			"\tlog_file_path:\t%s\n"
			"\tmax_clients:\t%d\n"
			"\tserver_port:\t%d\n",
			params.verb_level, params.daemonize, params.log_file_path,
			params.max_clients, params.server_port);

	clients = calloc(params.max_clients, sizeof(struct client*));
	if (clients == NULL) {
		perror("calloc");
		exit(-3);
	}

	sigemptyset(&sigact.sa_mask);
	sigact.sa_handler = handle_sigint;
	sigaction(SIGINT, &sigact, 0);

	logit(L_INFO "Starting server");

	start_server(&params);

	clean_up();

	return 0;
}

