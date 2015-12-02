#include <sys/stat.h>
#include <sys/types.h>
#include <sys/fcntl.h>
#include <sys/times.h>
#include <sys/errno.h>
#include <sys/time.h>
#include <stdint.h>

void _exit() {

}

int close(int file) {
	return -1;
}

char **environ; /* pointer to array of char * strings that define the current environment variables */

#include <errno.h>
#undef errno
int errno;
int execve(char *name, char **argv, char **env) {
	errno = ENOMEM;
	return -1;
}

int fork() {

	errno = EAGAIN;
	return -1;

}

int fstat(int file, struct stat *st) {

	st->st_mode = S_IFCHR;
	return 0;

}

int getpid() {

	return 1;

}

int isatty(int file) {

	return 1;

}

int kill(int pid, int sig) {

	errno = EINVAL;
	return -1;

}

int link(char *old, char *new) {

	errno = EMLINK;
	return -1;

}

int lseek(int file, int ptr, int dir) {

	return 0;

}

int open(const char *name, int flags, ...) {

	return -1;

}

int read(int file, char *ptr, int len) {

	return 0;

}

extern uint64_t handle_kernel_memory(int required_amount);

caddr_t sbrk(int incr) {
	return (caddr_t) handle_kernel_memory(incr);
}

int stat(const char *file, struct stat *st) {

	st->st_mode = S_IFCHR;
	return 0;

}

clock_t times(struct tms *buf) {

	return -1;

}

int unlink(char *name) {

	errno = ENOENT;
	return -1;

}

int wait(int *status) {
	errno = ECHILD;
	return -1;
}

int write(int file, char *ptr, int len) {
	return -1;

}
