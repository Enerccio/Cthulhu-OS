#include <sys/kernel.h>
#include <locale.h>

#ifdef __KCLIB_KERNEL_MODE

void __initialize_kclib(){

	// initialize locale
	__initialize_locale();

}

#endif
