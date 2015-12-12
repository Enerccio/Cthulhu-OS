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

#include <stddef.h>
#include <stdint.h>

/* _string_memops.c */
void memset(void* ptr, int32_t c, size_t n);
void* memcpy(void* restrict s1, const void* restrict s2, size_t n);
void* memmove(void* s1, const void* s2, size_t n);

/* _string_compops.c */
int memcmp(const void* s1, const void* s2, size_t n);
int strcmp(const char* s1, const char* s2);
int strcoll(const char* s1, const char* s2);
int strncmp(const char* s1, const char* s2, size_t n);
size_t strxfrm(char* restrict s1, const char* restrict s2,
		size_t n);

/* _string_string.c */ // yeah I know, very cool name
size_t strlen(const char *s);
char* strcpy(char* restrict s1, const char* restrict s2);

/* _string_searchops.c */
void* memchr(const void* s, int c, size_t n);
char* strchr(const char* s, int c);
size_t strcspn(const char* s1, const char* s2);
char* strpbrk(const char* s1, const char* s2);
char* strrchr(const char* s, int c);
size_t strspn(const char* s1, const char* s2);
char* strstr(const char* s1, const char* s2);
char* strtok(char* restrict s1, const char* restrict s2);
#ifdef __KCLIB_EXTENSIONS
char* strcpbrk(const char* s1, const char* s2);
#endif

#ifdef __cplusplus
}
#endif

#endif /* STRING_H_ */
