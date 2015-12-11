#include <string.h>
#include <stdlib.h>

char* strtok(char* restrict s1, const char* restrict s2){
	static char* __token;

	if (s2 == NULL)
		s2 = "";

	size_t slen = strlen(s2);
	if (s1 != NULL){
		while (1){
			if (*s1 == '\0')
				return NULL;
			bool found = false;
			for (size_t i=0; i<slen; i++)
				if (*s1 == s2[i]){
					found = true;
				}
			if (!found){
				++s1;
				continue;
			} else {
				__token = s1;
				break;
			}
		}
	}

	char* cpos = __token;
	while (1){
		if (__token == NULL || *__token == '\0'){
			return NULL;
		}

		for (size_t i=0; i<slen; i++)
			if (*__token == s2[i]){
				*__token = '\0';
				++__token;
				return cpos;
			}
		++__token;
	}
}
