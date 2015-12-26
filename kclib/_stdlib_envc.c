#include <stdlib.h>

char* getenv(const char* name){
#ifdef __KCLIB_KERNEL_MODE
	return NULL; // kernel has no environ
#else
	return __kclib_environ_search_u((char*)name);
#endif
}
