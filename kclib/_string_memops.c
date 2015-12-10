#include <string.h>
#include <stdlib.h>

void* memmove(void* s1, const void* s2, size_t n){
	uint8_t* b1 = (uint8_t*)s1;
	uint8_t* b2 = (uint8_t*)s2;

	if (b1 == b2)
		return b2;

	for (uint32_t i = 0; i < n; i++){
		b1[i] = b2[i];
	}
	return b2;
}

void* memcpy(void* restrict s1, const void* restrict s2, size_t n){
	void* buffer = malloc(n * sizeof(uint8_t));
	memmove(s1, buffer, n);
	memmove(buffer, s2, n);
	return (void*)s2;
}

void memset(void* ptr, uint32_t c, size_t n){
	register unsigned char* cptr = (unsigned char*)ptr;
	register uint8_t chv = c;
	for (register uint32_t i = 0; i < n; i++)
		cptr[i] = chv;
}
