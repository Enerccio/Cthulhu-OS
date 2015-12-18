#include <string.h>
#include <stdlib.h>
#include "intinc/string.h"

void* memchr(const void* s, int c, size_t n){
	register uint8_t ch = (uint8_t)c;
	register uint8_t* sb = (uint8_t*)s;
	for (register size_t i=0; i<n; i++){
		if (*sb == ch){
			return (void*)sb;
		}
		++sb;
	}
	return NULL;
}

char* strchr(const char* s, int c){
	register char* sb = (char*)s;
	register char ch = (char)c;
	do {
		if (*sb == ch)
			return sb;
	} while (*(sb++) != '\0');
	return NULL;
}

size_t strcspn(const char* s1, const char* s2){
	char* first_occ = strpbrk(s1, s2);
	if (first_occ == NULL)
		return strlen(s1);
	else
		return first_occ - s1;
}

char* strpbrk(const char* s1, register const char* s2){
	register char* sb = (char*)s1;
	size_t s2len = strlen(s2);
	do {
		for (register size_t i=0; i<s2len; i++){
			if (*sb == s2[i]){
				return sb;
			}
		}
	} while (*(sb++) != '\0');
	return NULL;
}

char* strrchr(const char* s, int c){
	char* last = NULL;
	char* test = (char*) s;

	while (test != NULL){
		test = strchr(test, c);
		if (test != NULL){
			last = test;
			++test;
		}
	}

	return last;
}

char* __strcpbrk(const char* s1, register const char* s2){
	register char* sb = (char*)s1;
	size_t s2len = strlen(s2);
	do {
		bool found = false;
		for (register size_t i=0; i<s2len; i++){
			if (*sb == s2[i]){
				found = true;
				break;
			}
		}
		if (!found){
			return sb;
		}
	} while (*(sb++) != '\0');
	return NULL;
}

#ifdef __KCLIB_EXTENSIONS
char* strcpbrk(const char* s1, const char* s2){
	return __strcpbrk(s1, s2);
}
#endif

size_t strspn(const char* s1, const char* s2){
	char* first_occ = __strcpbrk(s1, s2);
	if (first_occ == NULL)
		return strlen(s1);
	else if (first_occ == s1)
		return 0;
	else
		return (first_occ - s1);
}

char* strstr(const char* s1, const char* s2){
	size_t s2len = strlen(s2);

	if (s2len == 0)
		return (char*)s1;

	size_t s1len = strlen(s1);

	if (s2len > s1len)
		return NULL;

	for (size_t i=0; i<s1len-s2len; i++){
		if (strncmp(s1+i, s2, s2len) == 0){
			return (char*)s1+i;
		}
	}

	return NULL;
}

char* __strtok_ts(char** __token, char* restrict s1, const char* restrict s2){
	if (s2 == NULL)
		s2 = "";

	if (s1 != NULL)
		*__token = s1;

	if (*__token == NULL)
		return NULL;

	*__token = *__token + strspn(*__token, s2);

	char* pos = strpbrk(*__token, s2);
	if (pos == NULL){
		char* ret = *__token;
		*__token = NULL;
		return ret;
	}

	*pos = '\0';
	char* ret = *__token;
	*__token = pos+1;
	return ret;
}

char* strtok(char* restrict s1, const char* restrict s2){
	static char* __token = NULL;
	return __strtok_ts(&__token, s1, s2);
}
