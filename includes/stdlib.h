/*
 * stdlib.h
 *
 *  Created on: Dec 8, 2015
 *      Author: enerccio
 */

#ifndef STDLIB_H_
#define STDLIB_H_

#ifdef __cplusplus
extern "C" {
#endif

#define NULL ((void*)0)
#define EXIT_SUCCESS 0
#define EXIT_FAILURE 1

#ifdef KCLIB_KERNEL_MODE
#include <sys/external.h>
#endif

void* malloc(size_t size);
void* calloc(size_t nmemb, size_t size);
void* realloc(void *ptr, size_t size);
void  free(void *ptr);

#ifdef __cplusplus
}
#endif

#endif /* STDLIB_H_ */
