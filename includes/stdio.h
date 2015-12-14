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

/* _stdio_file.c */
int fclose(FILE* stream);
int fflush(FILE* stream);
/* _stdio_printf.c */

int vfprintf(FILE* restrict stream,
	const char* restrict format,
	va_list arg);

int fprintf(FILE* restrict stream,
		const char* restrict format, ...);


#ifdef __cplusplus
}
#endif

#endif /* STDIO_H_ */
