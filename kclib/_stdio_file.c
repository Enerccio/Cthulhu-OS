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

	return 0;
}

FILE* __buffered_io_stream(void* stream_descriptor){
	FILE* file = malloc(sizeof(FILE));

	return file;
}

void __initialize_streams(){
	stderr = __buffered_io_stream(__kclib_open_std_stream(__STDERR));
	stdout = __buffered_io_stream(__kclib_open_std_stream(__STDERR));
	stdin = __buffered_io_stream(__kclib_open_std_stream(__STDERR));
}
