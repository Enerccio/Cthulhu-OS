/*
 * stdio.h
 *
 *  Created on: Dec 8, 2015
 *      Author: enerccio
 */

#ifndef STDIO_H_
#define STDIO_H_

#ifdef __CDT_INDEXER__
#include <gcc_defs.h>
#endif

#ifdef __cplusplus
extern "C" {
#endif

#include <sys/external.h>
#include <stdarg.h>

#define SEEK_CUR 0
#define SEEK_END 1
#define SEEK_SET 2

typedef struct FILE FILE;

extern FILE* stderr;
extern FILE* stdout;
extern FILE* stdin;

#define EOF -1

/* _stdio_file.c */
int fclose(FILE* stream);
int fflush(FILE* stream);
FILE* fopen(const char* restrict filename,
		const char * restrict mode);
size_t fread(void* restrict ptr,
		size_t size, size_t nmemb,
		FILE* restrict stream);
size_t fwrite(const void* restrict ptr,
		size_t size, size_t nmemb,
		FILE* restrict stream);
int fseek(FILE *stream, long int offset, int whence);
long int ftell(FILE *stream);

#define BUFSIZ 2048

#define _IONBF 0
#define _IOFBF 1
#define _IOLBF 2

#define __FERROR_READ    1
#define __FERROR_WRITE   2
#define __FERROR_BUFFULL 3

#define __FORMAT_ERROR_NOT_ENOUGH_DATA -2
#define __FORMAT_ERROR_LENSPEC_INCORRECT -3
#define __FORMAT_ERROR_WRONG_SPECS -4

/* _stdio_buffer.h */
#define __BUF_ERROR_MALLOC_FAILURE 1
int setbuf(FILE* restrict stream, char* restrict buf);
int setvbuf(FILE* restrict stream, char* restrict buf, int mode, size_t size);

/* _stdio_printf.c */
int vfprintf(FILE* restrict stream,
	const char* restrict format,
	va_list arg);
int fprintf(FILE* restrict stream,
		const char* restrict format, ...);
int printf(const char* restrict format, ...);


#ifdef __cplusplus
}
#endif

#endif /* STDIO_H_ */
