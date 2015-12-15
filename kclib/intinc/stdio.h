/*
 * stdio.h
 *
 *  Created on: Dec 12, 2015
 *      Author: enerccio
 */

#ifndef INTINC_STDIO_H_
#define INTINC_STDIO_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <stdbool.h>

#define __STDOUT 1
#define __STDERR 2
#define __STDIN  3

#define __STDBUFFER_SIZE 2048
#define __BUF_FILES_STARTLEN 16

#define __FLAG_SETPOS    (1<<1)
#define __FLAG_CLOSEABLE (1<<2)
#define __FLAG_HASBUFFER (1<<3)

typedef struct {
	bool	 inited;
	uint8_t  mode;
	bool	 autoalloc;
	uint8_t* buffer;
	size_t   cpos;
	size_t   limit;
} __buffer_t;

struct FILE {
	void* handle;
	__buffer_t buffer;
	uint32_t fflags;
	size_t lastpos;

	bool closed;
};
typedef struct FILE FILE;

/* _stdio_file.c */
void __initialize_streams();
FILE* __create_filehandle(void* pd);

/* _stdio_buffer.h */
extern FILE** __buffered_handles;
extern size_t __buffered_handles_len;
size_t   __write_to_buffer(__buffer_t* buffer, uint8_t* data, size_t size);
size_t	 __buffer_maxsize(__buffer_t* buffer);
size_t	 __buffer_freesize(__buffer_t* buffer);
size_t	 __buffer_fseek(__buffer_t* buffer);
size_t	 __buffer_ftell(__buffer_t* buffer, size_t newpos);
uint8_t* __buffer_get_data(__buffer_t* buffer, size_t* len);
void     __free_buffer(__buffer_t* buffer);

#define __IS_CLOSEABLE(flags) ((flags & __FLAG_CLOSEABLE) == __FLAG_CLOSEABLE)
#define __IS_HASBUFFER(flags) ((flags & __FLAG_HASBUFFER) == __FLAG_HASBUFFER)

#ifdef __cplusplus
}
#endif

#endif /* INTINC_STDIO_H_ */
