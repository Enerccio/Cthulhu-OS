#include <stdlib.h>

#ifdef __KCLIB_KERNEL_MODE

// kernel mode does not support any of this shit

int atexit(void (*func)(void)){
	func = func;
	return -1;
}

void abort(void){

}

#else

#define __MAX_ATEXITS 64
uint16_t __atexit_func = 0;
void (*__atexit_funcs[__MAX_ATEXITS])(void);

int atexit(void (*func)(void)){
	if (__atexit_func >= __MAX_ATEXITS)
		return 1;
	__atexit_funcs[__atexit_func++] = func;
	return 0;
}

void abort(void){
	// TODO
}

#endif
