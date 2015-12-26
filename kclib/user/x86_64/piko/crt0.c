#include "../../../intinc/locale.h"
#include "../../../intinc/stdio.h"
#include "../../../intinc/string.h"
#include "../../../intinc/shmath.h"

#include <stdlib.h>
#include <sys/external.h>

extern int main(int argc, char** argv);

void _start(int argc, char** argv){
	__initialize_locale();
	__initialize_streams();

	int exitcode = main(argc, argv);

}
