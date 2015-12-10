#include <string.h>

size_t strlen(const char *s){
	size_t len;
	while (*s++ != '\0') ++len;
	return len;
}

char* strcpy(char* restrict s1, register const char* restrict s2){
	register char* t = s1;
	while ((*t++ = *s2++) != '\0') ;
	return s1;
}
