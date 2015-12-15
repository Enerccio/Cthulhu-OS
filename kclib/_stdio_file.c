#include <stdio.h>
#include <stdlib.h>
#include "intinc/stdio.h"

FILE __stderr;
FILE*  stderr;
FILE __stdout;
FILE*  stdout;
FILE __stdin;
FILE*  stdin;

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

FILE* __create_filehandle(void* pd){
	FILE* f = malloc(sizeof(FILE));
	if (f == NULL)
		return NULL;
	memset(f, 0, sizeof(FILE));
	f->handle = pd;
	return f;
}

void __init_std_stream(FILE* stream, void* fd, int mode){
	stream->handle = fd;
	setvbuf(stream, NULL, mode, BUFSIZ);
}

void __initialize_streams(){
	__buffered_handles = 0;
	__buffered_handles_len = 0;

	stdout = &__stdout;
	stderr = &__stderr;
	stdin  = &__stdin;

	memset(stdout, 0, sizeof(FILE));
	memset(stderr, 0, sizeof(FILE));
	memset(stdin, 0, sizeof(FILE));

	__init_std_stream(stdout, __kclib_open_std_stream(__STDOUT), _IOLBF);
	__init_std_stream(stderr, __kclib_open_std_stream(__STDERR), _IOLBF);
	__init_std_stream(stdin, __kclib_open_std_stream(__STDIN), _IONBF);
}

FILE* fopen(const char* restrict filename,
		const char * restrict mode){
#ifdef __KCLIB_KERNEL_MODE
	return NULL; // no support in kernel mode
#else
	FILE* fp = malloc(sizeof(FILE));
	if (fp == NULL)
		return NULL;

	memset(fp, 0, sizeof(FILE));

	return fp;
#endif
}
