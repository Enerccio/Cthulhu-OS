/*
 * _stdlib_rand.c
 *
 *  Created on: Dec 10, 2015
 *      Author: enerccio
 */


#include <stdlib.h>

uint32_t seed = 3;

int rand(){
	seed = (((seed * 214013L + 2531011L) >> 16) & 32767);
	return ((seed % RAND_MAX));
}

void srand(unsigned int _seed){
	seed = _seed + 2;
}
