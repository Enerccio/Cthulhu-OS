#include <stdlib.h>

int atoi(const char *nptr){
	return (int)strtol(nptr, (char **)NULL, 10);
}

long int strtol(const char* restrict nptr,
		char** restrict endptr,
		int base){

}
