#include "logger.h"

typedef enum {

	MESSAGE, WARNING, ERROR,

} log_level_t;

void log(log_level_t log_level, const char* message) {

	// write message that we are in the kernel
	kd_cwrite("MESSAGE: ", 0, 15);

	// display log level
	switch (log_level) {
	case WARNING:
		kd_cwrite("WARNING: ", 0, 6);
		break;
	case ERROR:
		kd_cwrite("ERROR: ", 0, 4);
		break;
	default:
		break;
	}

	kd_write(message);
	kd_put('\n');
}

void log_msg(const char* message) {
	log(MESSAGE, message);
}

void log_warn(const char* message) {
	log(WARNING, message);
}

void log_err(const char* message) {
	log(ERROR, message);
}
