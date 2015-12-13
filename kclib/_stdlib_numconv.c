#include <stdlib.h>
#include <limits.h>
#include <stdint.h>
#include <errno.h>

int atoi(const char *nptr){
	return (int)strtol(nptr, (char **)NULL, 10);
}

long int atol(const char *nptr){
	return strtoll(nptr, (char **)NULL, 10);
}

long int strtol(const char* restrict nptr,
		char** restrict endptr,
		int base){
	long long int base_result = strtoll(nptr, endptr, base);

}

long long int strtoll(const char* restrict nptr,
		char** restrict endptr,
		int base){



}
