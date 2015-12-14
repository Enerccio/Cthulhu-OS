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

typedef struct {
	uint8_t* buffer;
	size_t   cpos;
	size_t   limit;
	bool     resize;
} __buffer_t;

struct FILE {
	void* handle;
	__buffer_t buffer;
};

void __initialize_streams();

void     __initialize_buffer(__buffer_t* buffer, size_t initial_size, bool resizeable);
size_t   __write_to_buffer(__buffer_t* buffer, uint8_t* data, size_t size);

#ifdef __cplusplus
}
#endif

#endif /* INTINC_STDIO_H_ */
