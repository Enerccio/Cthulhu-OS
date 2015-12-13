#include <ctype.h>
#include <stdint.h>
#include <stdlib.h>
#include "intinc/locale.h"

int isspace(int c){
	lctype_h* ctype = __getlctype();
	return ctype->space_table[(unsigned char)c];
}

int isblank(int c){
	lctype_h* ctype = __getlctype();
	return ctype->blank_table[(unsigned char)c];
}

int isalnum(int c){
	return isalpha(c) || isdigit(c);
}

int isalpha(int c){
	return islower(c) || isupper(c);
}

int islower(int c){
	lctype_h* ctype = __getlctype();
	return ctype->lower_table[(unsigned char)c];
}

int isupper(int c){
	lctype_h* ctype = __getlctype();
	return ctype->upper_table[(unsigned char)c];
}

int isdigit(int c){
	lctype_h* ctype = __getlctype();
	return ctype->digit_table[(unsigned char)c];
}

int isxdigit(int c){
	lctype_h* ctype = __getlctype();
	return ctype->xdigit_table[(unsigned char)c];
}
