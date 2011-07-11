#ifndef _SERVER_H_
#define _SERVER_H_

#include <signal.h>
#include <pthread.h>

#define SERVER_VERSION "0.0.1"
#define DEFAULT_SERVER_PORT 1190
//#define MP_SEM_NAME "/main_process_sem"

struct parameters {
    int verb_level,
		daemonize,
		log_file,
		max_clients,
		server_port;

	char *log_file_path;
};

struct client {
	int socket;
	int master; /* if set to 1, client can control the robot */
	pthread_t thread_id;
};

void parse_cmdline(int argc, char *argv[], struct parameters *params);
void default_params(struct parameters *p);

void show_help(const char *path);
void show_version();

void clean_up();
void signal_handler(int sig, siginfo_t *siginfo, void *data);

void *serve_client(void *data);

int start_server(struct parameters *params);
int create_listen_socket(int port);

#endif

