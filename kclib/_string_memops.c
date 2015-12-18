#include <string.h>
#include <stdlib.h>
#include "intinc/string.h"

void* __memmove_cb(void* s1, const void* s2, size_t n, bool* flagged, char flag_char){
	unsigned char* dest = (unsigned char*) s1;
	const unsigned char* src = (const unsigned char*) s2;
	*flagged = 0;

	if ((uintptr_t)dest < (uintptr_t)src){
		for ( size_t i = 0; i < n; i++ ){
			dest[i] = src[i];
			if (flagged != NULL){
				if (src[i] == flag_char){
					*flagged = true;
				}
			}
		}
	} else {
		for ( size_t i = 0; i < n; i++ ){
			dest[n-(i+1)] = src[n-(i+1)];
			if (flagged != NULL){
				if (src[n-(i+1)] == flag_char){
					*flagged = true;
				}
			}
		}
	}
	return s1;
}

void* __memcpy_cb(void* restrict s1, const void* restrict s2, size_t n, bool* flagged, char flag_char){
	uint8_t* b1 = (uint8_t*)s1;
	uint8_t* b2 = (uint8_t*)s2;
	*flagged = 0;

	for (size_t i = 0; i < n; i++){
		b1[i] = b2[i];
		if (flagged != NULL){
			if (b2[i] == flag_char){
				*flagged = true;
			}
		}
	}
	return b2;
}

void* memmove(void* s1, const void* s2, size_t n){
	if (s1 == s2)
		return s1;

	unsigned char* dest = (unsigned char*) s1;
	const unsigned char* src = (const unsigned char*) s2;
	if ((uintptr_t)dest < (uintptr_t)src){
		for ( size_t i = 0; i < n; i++ )
			dest[i] = src[i];
	} else {
		for ( size_t i = 0; i < n; i++ )
			dest[n-(i+1)] = src[n-(i+1)];
	}
	return s1;
}

void* memcpy(void* restrict s1, const void* restrict s2, size_t n){
	uint8_t* b1 = (uint8_t*)s1;
	uint8_t* b2 = (uint8_t*)s2;

	if (b1 == b2)
		return b1;

	for (size_t i = 0; i < n; i++){
		b1[i] = b2[i];
	}
	return b2;
}

void memset(void* ptr, int32_t c, size_t n){
	register unsigned char* cptr = (unsigned char*)ptr;
	register uint8_t chv = c;
	for (register size_t i = 0; i < n; i++)
		cptr[i] = chv;
}
