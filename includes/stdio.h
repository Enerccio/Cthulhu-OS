/*
 * stdio.h
 *
 *  Created on: Dec 8, 2015
 *      Author: enerccio
 */

#ifndef STDIO_H_
#define STDIO_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <sys/external.h>

#define SEEK_CUR 0
#define SEEK_END 1
#define SEEK_SET 2



typedef struct FILE FILE;

extern FILE* stderr;
extern FILE* stdout;
extern FILE* stdin;

int fclose(FILE* stream);
int fflush(FILE* stream);


#ifdef __cplusplus
}
#endif

#endif /* STDIO_H_ */
