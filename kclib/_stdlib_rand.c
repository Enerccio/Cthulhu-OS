#include <stdlib.h>
#include <stdarg.h>

uint32_t __seed = 3;

int rand(){
	__seed = (((__seed * 214013L + 2531011L) >> 16) & 32767);
	return ((__seed % RAND_MAX));
}

void srand(unsigned int ___seed){
	__seed = ___seed + 2;
}
