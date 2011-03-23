#ifndef _SERVER_H_
#define _SERVER_H_

#define SERVER_VERSION "0.0.1"
#define MAX_CLIENTS 10
#define SERVER_PORT 1190

struct parameters {
    int verb_level;
    int daemonize;
	int log_file;

	char *log_file_path;
};

void parse_cmdline(int argc, char *argv[], struct parameters *params);
void default_params(struct parameters *p);

void show_help(const char *path);
void show_version();

void handle_sigint(int sig);

void serve_client(int sock);

int start_server(struct parameters *params);
int create_listen_socket(int port);

#endif

