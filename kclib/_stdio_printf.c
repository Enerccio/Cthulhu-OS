#include <stdio.h>

int fprintf(FILE* restrict stream,
		const char* restrict format, ...){
	va_list args;
	va_start(args, format);
	int retval = vfprintf(stderr, format, args);
	va_end(args);
	return retval;
}

int vfprintf(FILE* restrict stream,
		const char* restrict format,
		va_list arg){

}
