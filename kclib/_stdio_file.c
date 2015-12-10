#include <stdio.h>

struct FILE {
	void* handle;
};


int fclose(FILE* stream){
#ifdef __KCLIB_KERNEL_MODE
	// kernel has no files
	return 0;
#endif
	// TODO
	return 0;
}

int fflush(FILE* stream){
#ifdef __KCLIB_KERNEL_MODE
	// kernel has no files
	return 0;
#endif
	// TODO
	return 0;
}
