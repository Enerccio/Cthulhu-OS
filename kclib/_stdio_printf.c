#include <stdio.h>

int printf(const char* restrict format, ...){
	va_list args;
	va_start(args, format);
	int retval = vfprintf(stdout, format, args);
	va_end(args);
	return retval;
}

int fprintf(FILE* restrict stream,
		const char* restrict format, ...){
	va_list args;
	va_start(args, format);
	int retval = vfprintf(stream, format, args);
	va_end(args);
	return retval;
}

int vfprintf(FILE* restrict stream,
		const char* restrict format,
		va_list arg){

	return 0;
}
