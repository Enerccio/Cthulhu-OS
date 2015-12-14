#include <stdio.h>
#include <stdlib.h>
#include "intinc/stdio.h"

FILE* stderr;
FILE* stdout;
FILE* stdin;

int fclose(FILE* stream){
	fflush(stream);
	free(stream);
	return 0;
}

int fflush(FILE* stream){
	if (stream == NULL){
		int retv = 0;
		retv |= fflush(stderr);
		retv |= fflush(stdout);
		retv |= fflush(stdin);
		return retv;
	}



	return 0;
}

FILE* __buffered_io_stream(void* stream_descriptor, size_t buffer_size){
	FILE* file = malloc(sizeof(FILE));
	__initialize_buffer(&file->buffer, buffer_size, false);
	return file;
}

void __initialize_streams(){
	stderr = __buffered_io_stream(__kclib_open_std_stream(__STDERR), __STDBUFFER_SIZE);
	stdout = __buffered_io_stream(__kclib_open_std_stream(__STDOUT), __STDBUFFER_SIZE);
	stdin = __buffered_io_stream(__kclib_open_std_stream(__STDIN), 1);
}
