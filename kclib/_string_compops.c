#include <string.h>

int memcmp(const void* s1, const void* s2, size_t n){
	register uint8_t* b1 = (uint8_t*)s1;
	register uint8_t* b2 = (uint8_t*)s2;
	for (register uint32_t i = 0; i<n; i++){
		int diff = ((int)b1[i]) - ((int)b2[i]);
		if (diff != 0)
			return diff;
	}
	return 0;
}

int strcmp(const char* s1, const char* s2){
	register uint8_t* b1 = (uint8_t*)s1;
	register uint8_t* b2 = (uint8_t*)s2;
	uint8_t a, b;
	while (1){
		int diff = ((int)(a=*b1++)) - ((int)(b=*b2++));
		if (diff != 0)
			return diff;
		if ((a == 0) && (b == 0))
			return 0;
	}
}

int strcoll(const char* s1, const char* s2){

}

int strncmp(const char* s1, const char* s2, size_t n){
	register uint8_t* b1 = (uint8_t*)s1;
	register uint8_t* b2 = (uint8_t*)s2;
	uint8_t a, b;
	while (1){
		int diff = ((int)(a=*b1++)) - ((int)(b=*b2++));
		if (diff != 0)
			return diff;
		if ((a == 0) && (b == 0))
			return 0;
		--n;
		if (n == 0)
			return 0;
	}
}

size_t strxfrm(char* restrict s1, const char* restrict s2,
		size_t n){

}
