#include <stdlib.h>
#include <stdio.h>

#ifdef __KCLIB_KERNEL_MODE

// kernel mode does not support any of this shit

int atexit(void (*func)(void)){
	return -1;
}

void abort(void){

}

void exit(int status){

}

#else

typedef void (*__atexit_funcall)(void);
#define __MAX_ATEXITS 64
uint16_t __atexit_func = 0;
__atexit_funcall __atexit_funcs[__MAX_ATEXITS];

int atexit(void (*func)(void)){
	if (__atexit_func >= __MAX_ATEXITS)
		return -1;
	__atexit_funcs[__atexit_func++] = func;
	return 0;
}

void abort(void){
	__kclib_terminate(-1);
	while (true) ; /* sentinel loop */
}

void exit(int status){
	for (uint16_t ate = 0; ate < __atexit_func; ate++)
		__atexit_funcs[ate]();

	fflush(NULL);

	__kclib_terminate(status);
	while (true) ; /* sentinel loop */
}

#endif
