#include <stdio.h>
#include "intinc/stdio.h"
#include <stdlib.h>
#include <string.h>
#include "intinc/shmath.h"


FILE** __buffered_handles;
size_t __buffered_handles_len;

size_t __write_to_buffer(__buffer_t* buffer, uint8_t* data, size_t size){
	size_t rembytes = buffer->limit - buffer->cpos;

	if (rembytes == 0){
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

uint8_t* __buffer_get_data(__buffer_t* buffer, size_t* len){
	*len = buffer->cpos;
	return buffer->buffer;
}

void __free_buffer(__buffer_t* buffer){
	if (buffer->autoalloc){
		free(buffer->buffer);
		buffer->buffer = NULL;
		buffer->limit = 0;
		buffer->cpos = 0;
	} else {
		buffer->buffer = NULL;
		buffer->limit = 0;
		buffer->cpos = 0;
	}
}

int setvbuf(FILE* restrict stream, char* restrict buf, int mode, size_t size){
	if (stream->buffer.inited){
		__free_buffer(&stream->buffer);
		memset(&stream->buffer, 0, sizeof(__buffer_t));
	}

	stream->buffer.mode = mode;
	stream->buffer.inited = true;

	stream->buffer.cpos = 0;
	stream->buffer.limit = 0;

	if (stream->buffer.mode != _IONBF){
		stream->fflags |= __FLAG_HASBUFFER;
		if (buf == NULL){
			buf = malloc(size);
			if (buf == NULL)
				return __BUF_ERROR_MALLOC_FAILURE;
			stream->buffer.autoalloc = true;
		} else {
			stream->buffer.autoalloc = false;
		}
		stream->buffer.buffer = (uint8_t*)buf;
		stream->buffer.limit = size;

		if (__buffered_handles_len == 0){
				__buffered_handles_len = __BUF_FILES_STARTLEN;
				__buffered_handles = malloc(sizeof(FILE)*__buffered_handles_len);
				__buffered_handles[0] = stream;
			} else {
				bool inserted = false;
		reinsert:
				for (size_t i = 0; i < __buffered_handles_len; i++){
					if (__buffered_handles[i] == NULL){
						__buffered_handles[i] = stream;
						inserted = true;
						break;
					}
				}
				if (!inserted){
					void* nptr = realloc(__buffered_handles, __buffered_handles_len*2);
					if (nptr == NULL)
						return 1;
					memset(nptr+(sizeof(FILE*)*__buffered_handles_len), 0, sizeof(FILE*)*__buffered_handles_len);
					__buffered_handles = nptr;
					__buffered_handles_len *= 2;
					goto reinsert;
				}
			}
	}

	return 0;
}

int setbuf(FILE* restrict stream, char* restrict buf){
	return setvbuf(stream, buf, _IOFBF, BUFSIZ);
}
