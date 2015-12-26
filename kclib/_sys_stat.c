#include <sys/stat.h>
#include <sys/external.h>
#include <errno.h>

int fstat(int fdescriptor, struct stat* stat){
#ifdef __KCLIB_KERNEL_MODE
	errno = EIO;
	return -1;
#else
	return __kclib_fstat_u((void*)(intptr_t)fdescriptor, stat, &errno);
#endif
}
