#include <ctype.h>
#include <stdint.h>
#include <stdlib.h>
#include "intinc/locale.h"

int isspace(int c){
	lctype_h* ctype = __getlctype();
	return ctype->space_table[(unsigned char)c];
}
