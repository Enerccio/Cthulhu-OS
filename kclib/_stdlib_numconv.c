#include <stdlib.h>
#include <stdbool.h>
#include <stdint.h>
#include <limits.h>
#include <stdint.h>
#include <errno.h>
#include <ctype.h>

#if __GNUC__ > 5
#define __WILL_OVERFLOW_ADD(a, b, where) __builtin_add_overflow(a, b, &where)
#define __WILL_OVERFLOW_MUL(a, b, where) __builtin_mul_overdlow(a, b, &where)
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
	return strtoll(nptr, (char **)NULL, 10);
}

const char* __filter(const char* nptr){
	char c;
	do {
		c = *(nptr++);
	} while (c == '\0' || isspace(c));
	return nptr;
}

size_t __countnumeric(const char* ptr, int* base, bool* minus, const char** newstart){
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
		if (blen > 2 && (strcmp(ptr, "0x") == 0 || strcmp(ptr, "0X") == 0)){
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
		if (strcmp(ptr, "0x") == 0 || strcmp(ptr, "0X") == 0){
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

	*newstart = ptr;
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

uintmax_t __strtomax(const char* restrict nptr, int base, bool* overflow, bool* sign, const char** remainder){
	const char* start = __filter(nptr);
	size_t safelen = __countnumeric(start, &base, sign, &start);

	if (remainder != NULL){
		*remainder = start;
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

}

long long int strtoll(const char* restrict nptr,
		char** restrict endptr,
		int base){



}
