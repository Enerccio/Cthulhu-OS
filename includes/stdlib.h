#ifndef STDLIB_H_
#define STDLIB_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <stdbool.h>
#include <string.h>
#include <assert.h>
#include <stdint.h>

#define NULL (0)
#define EXIT_SUCCESS 0
#define EXIT_FAILURE 1

#include <sys/external.h>

/* _stdlib_alloc.c */
void* malloc(size_t size);
void* calloc(size_t nmemb, size_t size);
void* realloc(void* ptr, size_t size);
void  free(void* ptr);

/* _stdlib_rand.c */
#define RAND_MAX 65535
int rand();
void srand(unsigned int seed);

/* _stdlib_exit.c */
int atexit(void (*func)(void));
void abort(void);

/* _stdlib_searchops.c */
void* bsearch(const void* key, const void* base,
		size_t nmemb, size_t size,
		int (*compar)(const void *, const void *));
void qsort(void* base, size_t nmemb, size_t size,
		int (*compar)(const void *, const void *));

/* _stdlib_numconv.c */
double atof(const char *nptr);
int atoi(const char *nptr);
long int atol(const char *nptr);
long long int atoll(const char *nptr);
double strtod(const char* restrict nptr,
		char** restrict endptr);
float strtof(const char* restrict nptr,
		char** restrict endptr);
long double strtold(const char* restrict nptr,
		char** restrict endptr);
long int strtol(const char* restrict nptr,
		char** restrict endptr,
		int base);
long long int strtoll(const char* restrict nptr,
		char** restrict endptr,
		int base);
unsigned long int strtoul(const char* restrict nptr,
		char** restrict endptr,
		int base);
unsigned long long int strtoull(const char* restrict nptr,
		char** restrict endptr,
		int base);


#ifdef __cplusplus
}
#endif

#endif /* STDLIB_H_ */
