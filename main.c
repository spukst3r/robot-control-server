#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <sys/socket.h>
#include <semaphore.h>
#include <fcntl.h>
#include <unistd.h>
#include <signal.h>
#include <getopt.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "server.h"
#include "log.h"
#include "config.h"

struct parameters params;
struct client **clients;
int listen_socket;
sem_t *main_process_sem;

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
				arg = atoi(optarg);
				params->verb_level = arg;
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
				params->server_port = arg;
				break;

			case 'M':
				arg = atoi(optarg);
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
	char *path = DEFAULT_LOG_FILE;
	p->log_file_path = malloc(strlen(path) + 1);

	strcpy(p->log_file_path, path);

	p->verb_level  = 5;
	p->max_clients = 10;
	p->server_port = DEFAULT_SERVER_PORT;
}

void show_help(const char *path)
{
	printf("Usage:\n"
			"%s [OPTION]...\n", path);
	printf("  -h, --help              display this help and exit\n"
			"  -V, --version           display version information and exit\n"
			"  -v, --verbosity=LEVEL   set log verbosity level (from 0 to 5)\n"
			"  -b, --daemonize         fork to background immediatly\n"
			"  -l. --log-file=FILE     set log file location, '-' for logging to stdout\n"
			"                            %s by default\n"
			"  -p, --port=NUM          set server listening port to NUM\n"
			"                            %d by default\n"
			"  -M, --max-clients=NUM   set max clients count to NUM\n"
			"                            %d by default\n",
			DEFAULT_LOG_FILE, DEFAULT_SERVER_PORT, 10);
}

void show_version()
{
	printf("%s\n", SERVER_VERSION);
}

void clean_up()
{
	int i;
	static int cleaned = 0;

	if (cleaned)
		return;

	logit(L_DEBUG "Cleaning up...");

	if (clients)
		for (i=0; i<params.max_clients; i++) {
			if (clients[i]) {
				pthread_cancel(clients[i]->thread_id);
				shutdown(clients[i]->socket, SHUT_RDWR);
				close(clients[i]->socket);
				free(clients[i]);
				clients[i] = NULL;
			}
		}

	shutdown(listen_socket, SHUT_RDWR);
	close(listen_socket);
	free(clients);

	logit(L_DEBUG "Done");

	close(params.log_file);
	free(params.log_file_path);

	log_dispose();

	cleaned = 1;
}

void signal_handler(int sig, siginfo_t *siginfo, void *data)
{
	logit(L_INFO "%s\n", strsignal(sig));

	switch (sig) {
		case SIGCHLD:
			logit(L_INFO "Child [pid: %d] exited", siginfo->si_pid);
			break;

		default:
			logit(L_INFO "Exiting");
			clean_up();
			exit(0);
	}
}

int main(int argc, char *argv[])
{
	struct sigaction sigact;

	atexit(clean_up);

	if (log_init() < 0) {
		fprintf(stderr, "Log system failed to start, aborting\n");
		exit (-1);
	}

	default_params(&params);

	if (parse_config("rcsrc", &params))
		return -1;

	parse_cmdline(argc, argv, &params);
	if (check_config(&params)) {
		show_help(argv[0]);
		return -1;
	}

	if (strcmp(params.log_file_path, "-") == 0)
		params.log_file = 2; //stderr
	else
		params.log_file = open(params.log_file_path, O_CREAT | O_TRUNC | O_RDWR, 0640);

	if (params.log_file < 0) {
		perror("open()");
		params.log_file = 1; //stdout
		logit(L_WARNING "Failed to open log file for writing, logging to stdout...");
	}

	logit(L_DEBUG "Startup parameters:\n"
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

	sigact.sa_flags     = SA_SIGINFO;
	sigact.sa_sigaction = signal_handler;

	sigaction(SIGINT, &sigact, NULL);
	sigaction(SIGTERM, &sigact, NULL);
	sigaction(SIGCHLD, &sigact, NULL);

	logit(L_INFO "Starting server");

	start_server(&params);

	clean_up();

	return 0;
}

