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

#include <stdbool.h>
#include <string.h>
#include <assert.h>

#define NULL (0)
#define EXIT_SUCCESS 0
#define EXIT_FAILURE 1

#include <sys/external.h>

void* malloc(size_t size);
void* calloc(size_t nmemb, size_t size);
void* realloc(void* ptr, size_t size);
void  free(void* ptr);

#ifdef __cplusplus
}
#endif

#endif /* STDLIB_H_ */
