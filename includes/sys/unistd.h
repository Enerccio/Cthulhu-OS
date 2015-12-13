/*
 * unistd.h
 *
 *  Created on: Dec 12, 2015
 *      Author: enerccio
 */

#ifndef SYS_UNISTD_H_
#define SYS_UNISTD_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <sys/types.h>

typedef id_t pid_t;

pid_t fork(void);
int execv(const char*, char* const []);
int execve(const char*, char* const [], char* const []);
int execvp(const char*, char* const []);

#ifdef __cplusplus
}
#endif

#endif /* SYS_UNISTD_H_ */
