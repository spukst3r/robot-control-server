#ifndef _LOG_H_
#define _LOG_H_

#define L_FATAL 	"FATAL: "
#define L_ERROR 	"ERROR: "
#define L_WARNING	"WARNING: "
#define L_INFO	 	"INFO: "
#define L_DEBUG 	"DEBUG: "

int logit(const char *format, ...);
int log_init();
int log_dispose();

#endif

