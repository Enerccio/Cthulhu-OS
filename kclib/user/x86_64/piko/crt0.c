#include "../../../intinc/locale.h"
#include "../../../intinc/stdio.h"
#include "../../../intinc/string.h"
#include "../../../intinc/shmath.h"

#include <stdlib.h>
#include <sys/external.h>

extern int main(int argc, char** argv);
extern void _init();
extern void _finit();

void _start(int argc, char** argv){
	__initialize_locale();
	__initialize_streams();

	_init();

	int exitcode = main(argc, argv);

	_finit();

	exit(exitcode);
}
