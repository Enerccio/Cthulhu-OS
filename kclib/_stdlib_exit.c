#include <stdlib.h>

#define __MAX_ATEXITS 64
uint16_t __atexit_func = 0;
void (*__atexit_funcs[__MAX_ATEXITS])(void);

int atexit(void (*func)(void)){
	if (__atexit_func >= __MAX_ATEXITS)
		return 1;
	__atexit_funcs[__atexit_func++] = func;
	return 0;
}
