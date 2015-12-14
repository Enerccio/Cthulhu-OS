#include <stdio.h>
#include <stdlib.h>
#include "intinc/stdio.h"

FILE __stderr;
FILE*  stderr;
FILE __stdout;
FILE*  stdout;
FILE __stdin;
FILE*  stdin;

FILE** __buffered_handles;
size_t __buffered_handles_len;

int fclose(FILE* stream){
	if (!__IS_CLOSEABLE(stream->fflags))
		return 0;

	int err = fflush(stream);
	__free_buffer(&stream->buffer);
	if (__IS_HASBUFFER(stream->fflags)){
		for (size_t i=0; i<__buffered_handles_len; i++){
			if (__buffered_handles[i] == stream)
				__buffered_handles[i] = NULL;
		}
	}
	return err;
}

int fflush(FILE* stream){
	if (stream == NULL){
		int retv = 0;
		for (size_t i=0; i<__buffered_handles_len; i++){
			if (__buffered_handles[i] != NULL)
				retv |= fflush(__buffered_handles[i]);
		}
		return retv;
	}

	if (__IS_CLOSEABLE(stream->fflags) && stream->closed)
		return EOF;
	if (!__IS_HASBUFFER(stream->fflags)){
		return 0;
	}

	size_t len;
	uint8_t* data = __buffer_get_data(&stream->buffer, &len);

	ptrdiff_t datasent = __kclib_send_data(stream->handle, data, len);
	if (datasent == EOF){
		return 1;
	}

	__buffer_ftell(&stream->buffer, 0);

	if ((size_t)datasent < len){
		__write_to_buffer(&stream->buffer, data+datasent, len-datasent);
		return 1;
	} else {
		return 0;
	}
}

int __buffered_io_stream(FILE* file, void* stream_descriptor, size_t buffer_size){
	memset(file, 0, sizeof(FILE));
	file->handle = stream_descriptor;
	__initialize_buffer(&file->buffer, buffer_size, false);
	if (__buffer_freesize(&file->buffer) > 0){
		file->fflags |= __FLAG_HASBUFFER;
		if (__buffered_handles_len == 0){
			__buffered_handles_len = __BUF_FILES_STARTLEN;
			__buffered_handles = malloc(sizeof(FILE)*__buffered_handles_len);
			__buffered_handles[0] = file;
		} else {
			bool inserted = false;
	reinsert:
			for (size_t i = 0; i < __buffered_handles_len; i++){
				if (__buffered_handles[i] == NULL){
					__buffered_handles[i] = file;
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

void __std_stream(FILE* file){

}

void __initialize_streams(){
	__buffered_handles = 0;
	__buffered_handles_len = 0;

	stdout = &__stdout;
	stderr = &__stderr;
	stdin  = &__stdin;

	__buffered_io_stream(stderr, __kclib_open_std_stream(__STDERR), __STDBUFFER_SIZE);
	__buffered_io_stream(stdout, __kclib_open_std_stream(__STDOUT), __STDBUFFER_SIZE);
	__buffered_io_stream(stdin, __kclib_open_std_stream(__STDIN), 0);

	__std_stream(stdout);
	__std_stream(stderr);
	__std_stream(stdin);
}
