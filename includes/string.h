/*
 * string.h
 *
 *  Created on: Dec 8, 2015
 *      Author: enerccio
 */

#ifndef STRING_H_
#define STRING_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>

typedef uint32_t size_t;

/* _string_memops.c */
void memset(void* ptr, uint32_t c, size_t n);
void* memcpy(void* restrict s1, const void* restrict s2, size_t n);
void* memmove(void* s1, const void* s2, size_t n);

/* _string_string.c */ // yeah I know, very cool name
size_t strlen(const char *s);
char* strcpy(char* restrict s1, const char* restrict s2);

#ifdef __cplusplus
}
#endif

#endif /* STRING_H_ */
