#include <stdio.h>
#include "intinc/stdio.h"
#include <stdlib.h>
#include <string.h>
#include "intinc/shmath.h"

void __initialize_buffer(__buffer_t* buffer, size_t initial_size, bool resize){
	memset(buffer, 0, sizeof(__buffer_t));
	buffer->cpos = 0;
	buffer->limit = initial_size;
	buffer->resize = resize;
	buffer->buffer = malloc(sizeof(uint8_t)*buffer->limit);
	if (buffer->buffer == NULL)
		buffer->limit = 0;
}

int __resize_buffer(__buffer_t* buffer){
	size_t old_limit = buffer->limit;
	uint8_t* old_ptr = buffer->buffer;

	buffer->limit *= 2;
	buffer->buffer = realloc(buffer->buffer, buffer->limit);

	if (buffer->buffer == NULL){
		buffer->limit = old_limit;
		buffer->buffer = old_ptr;
		return 1;
	}
	return 0;
}

size_t __write_to_buffer(__buffer_t* buffer, uint8_t* data, size_t size){
	size_t rembytes = buffer->limit - buffer->cpos;

	if (rembytes == 0 && buffer->resize){
		if (__resize_buffer(buffer)){
			return 0;
		}
		return __write_to_buffer(buffer, data, size);
	} else if (rembytes == 0 && !buffer->resize){
		// buffer full, no resize
		return 0;
	}

	size_t awbytes;
	size_t ltwbytes;
	if (size > rembytes){
		awbytes = rembytes;
		ltwbytes = size - rembytes;
	} else {
		awbytes = size;
		ltwbytes = 0;
	}

	memmove(buffer->buffer + buffer->cpos, data, awbytes);
	buffer->cpos += awbytes;

	if (ltwbytes != 0){
		awbytes += __write_to_buffer(buffer, data+awbytes, ltwbytes);
	}
	return awbytes;
}

size_t __buffer_maxsize(__buffer_t* buffer){
	return buffer->limit;
}

size_t __buffer_freesize(__buffer_t* buffer){
	return buffer->limit - buffer->cpos;
}

size_t __buffer_fseek(__buffer_t* buffer){
	return buffer->cpos;
}

size_t __buffer_ftell(__buffer_t* buffer, size_t newpos){
	if (newpos >= buffer->limit)
		return buffer->cpos;
	buffer->cpos = newpos;
	return newpos;
}

void __free_buffer(__buffer_t* buffer){
	free(buffer->buffer);
}

uint8_t* __buffer_get_data(__buffer_t* buffer, size_t* len){
	*len = buffer->cpos;
	return buffer->buffer;
}
