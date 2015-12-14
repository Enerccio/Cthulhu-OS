#include <stdlib.h>
#include <stdbool.h>
#include <stdint.h>
#include <limits.h>
#include <stdint.h>
#include <errno.h>
#include <ctype.h>

#if __GNUC__ >= 5
#define __WILL_OVERFLOW_ADD(a, b, where) __builtin_add_overflow(a, b, where)
#define __WILL_OVERFLOW_MUL(a, b, where) __builtin_mul_overflow(a, b, where)
#else
bool __safe_addition(uintmax_t a, uintmax_t b, uintmax_t* place){
	if (a > (UINTMAX_MAX - b)) {
		*place = 0;
		return true;
	} else {
		*place = a+b;
		return false;
	}
}
#define __WILL_OVERFLOW_ADD(a, b, where) __safe_addition(a, b, where)
bool __safe_multiplication(uintmax_t a, uintmax_t b, uintmax_t* place){
	if (a > UINTMAX_MAX / b) {
		*place = 0;
		return true;
	} else {
		*place = a * b;
		return false;
	}
}
#define __WILL_OVERFLOW_MUL(a, b, where) __safe_multiplication(a, b, where)
#endif

int atoi(const char *nptr){
	return (int)strtol(nptr, (char **)NULL, 10);
}

long int atol(const char *nptr){
	return strtol(nptr, (char **)NULL, 10);
}

long long int atoll(const char *nptr){
	return strtoll(nptr, (char **)NULL, 10);
}

const char* __filter(const char* nptr){
	char c;
	do {
		c = *(nptr++);
	} while (c == '\0' || isspace(c));
	if (!c == '\0' && !isspace(c))
		--nptr;
	return nptr;
}

size_t __countnumeric(const char** ptraddr, int* base, bool* minus, const char** end){
	const char* ptr = *ptraddr;

	size_t blen = strlen(ptr);
	if (blen == 0)
		return 0;
	if (blen > 0){
		if (ptr[0] == '+' || ptr[0] == '-'){
			if (ptr[0] == '-')
				*minus = true;
			else
				*minus = false;
			++ptr;
			--blen;
		}
	}

	bool detected_base = false;
	if (*base == 0){
		// detect base
		*base = 10; // default base
		if (blen > 2 && ((strncmp(ptr, "0x", 2) == 0 || strncmp(ptr, "0X", 2) == 0))){
			ptr += 2;
			blen -= 2;
			*base = 16;
			detected_base = true;
		} else if (blen > 1 && ptr[0] == '0'){
			++ptr;
			--blen;
			*base = 8;
			detected_base = true;
		}

	}

	if (!detected_base && *base == 16 && blen > 2){
		if (strncmp(ptr, "0x", 2) == 0 || strncmp(ptr, "0X", 2) == 0){
			ptr += 2;
			blen -= 2;
		}
	}

	if (!detected_base && *base == 8 && blen > 1){
		if (ptr[0] == '0'){
			++ptr;
			--blen;
		}
	}

	*ptraddr = ptr;

	size_t validlen = 0;
	static char detection_radix_upper[] = {
			'0', '1', '2', '3', '4', '5', '6', '7', '8', '9',
			'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H', 'I', 'J', 'K', 'L', 'M', 'N', 'O', 'P', 'Q', 'R', 'S', 'T', 'U', 'V', 'W', 'X', 'Y', 'Z'
	};
	static char detection_radix_lower[] = {
			'0', '1', '2', '3', '4', '5', '6', '7', '8', '9',
			'a', 'b', 'c', 'd', 'e', 'f', 'g', 'h', 'i', 'j', 'k', 'l', 'm', 'n', 'o', 'p', 'q', 'r', 's', 't', 'u', 'v', 'w', 'x', 'y', 'z'
	};
	while (blen != 0){
		char c = *ptr;
		bool valid = false;
		for (uint8_t i = 0; i<*base; i++){
			if (detection_radix_upper[i] == c || detection_radix_lower[i] == c){
				valid = true;
				break;
			}
		}

		if (!valid)
			break;

		++validlen;
		--blen;
		++ptr;
	}

	*end = ptr;
	return validlen;
}

uintmax_t __radixval(char c){
	static char val_radix_upper[] = {
					'0', '1', '2', '3', '4', '5', '6', '7', '8', '9',
					'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H', 'I', 'J', 'K', 'L', 'M', 'N', 'O', 'P', 'Q', 'R', 'S', 'T', 'U', 'V', 'W', 'X', 'Y', 'Z'
			};
	static char val_radix_lower[] = {
			'0', '1', '2', '3', '4', '5', '6', '7', '8', '9',
			'a', 'b', 'c', 'd', 'e', 'f', 'g', 'h', 'i', 'j', 'k', 'l', 'm', 'n', 'o', 'p', 'q', 'r', 's', 't', 'u', 'v', 'w', 'x', 'y', 'z'
	};
	for (int i=0; i<36; i++)
		if (c == val_radix_upper[i] || c == val_radix_lower[i])
			return i;
	return 0;
}

uintmax_t __strtomax(const char* restrict nptr, int base, bool* overflow, bool* sign, char** restrict remainder){
	const char* start = __filter(nptr);
	const char* end;
	size_t safelen = __countnumeric(&start, &base, sign, &end);

	if (remainder != NULL){
		*remainder = (char*)end;
	}

	uintmax_t result = 0;

	if (safelen == 0){
		return 0;
	}

	uintmax_t multiplier = 1;
	do {
		--safelen;
		char c = start[safelen];
		uintmax_t val = __radixval(c);
		uintmax_t cval;

		if (__WILL_OVERFLOW_MUL(val, multiplier, &cval)){
			*overflow = true;
			return UINTMAX_MAX;
		}

		if (__WILL_OVERFLOW_ADD(cval, result, &result)){
			*overflow = true;
			return UINTMAX_MAX;
		}

		if (safelen != 0){
			if (__WILL_OVERFLOW_MUL((uintmax_t)base, multiplier, &multiplier)){
				*overflow = true;
				return UINTMAX_MAX;
			}
		}

	} while (safelen > 0);

	return result;
}

long int strtol(const char* restrict nptr,
		char** restrict endptr,
		int base){
	bool overflow = false, sign = false;
	uintmax_t pval = __strtomax(nptr, base, &overflow, &sign, endptr);

	if (overflow || pval > LONG_MAX){
		errno = ERANGE;
		if (sign){
			return LONG_MIN;
		} else {
			return LONG_MAX;
		}
	} else {
		if (sign)
			return 0 - (long int)pval;
		else
			return (long int)pval;
	}
}

long long int strtoll(const char* restrict nptr,
		char** restrict endptr,
		int base){
	bool overflow = false, sign = false;
	uintmax_t pval = __strtomax(nptr, base, &overflow, &sign, endptr);

	if (overflow || pval > LLONG_MAX){
		errno = ERANGE;
		if (sign){
			return LLONG_MIN;
		} else {
			return LLONG_MAX;
		}
	} else {
		if (sign)
			return 0 - (long long int)pval;
		else
			return (long long int)pval;
	}
}

unsigned long int strtoul(const char* restrict nptr,
		char** restrict endptr,
		int base){
	bool overflow = false, sign = false;
	uintmax_t pval = __strtomax(nptr, base, &overflow, &sign, endptr);

	if (sign){
		errno = ERANGE;
		return 0;
	}

	if (overflow || pval > LONG_MAX){
		errno = ERANGE;
		return LONG_MAX;
	} else {
		return (long int)pval;
	}
}

unsigned long long int strtoull(const char* restrict nptr,
		char** restrict endptr,
		int base){
	bool overflow = false, sign = false;
	uintmax_t pval = __strtomax(nptr, base, &overflow, &sign, endptr);

	if (sign){
		errno = ERANGE;
		return 0;
	}

	if (overflow || pval > LLONG_MAX){
		errno = ERANGE;
		return LLONG_MAX;
	} else {
		return (long long int)pval;
	}
}
