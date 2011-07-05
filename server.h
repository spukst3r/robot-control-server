#ifndef _SERVER_H_
#define _SERVER_H_

#define SERVER_VERSION "0.0.1"
#define DEFAULT_SERVER_PORT 1190

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
};

void parse_cmdline(int argc, char *argv[], struct parameters *params);
void default_params(struct parameters *p);

void show_help(const char *path);
void show_version();

void clean_up();
void handle_sigint(int sig);

void serve_client(int sock);

int start_server(struct parameters *params);
int create_listen_socket(int port);

#endif

