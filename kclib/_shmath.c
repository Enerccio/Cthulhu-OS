#include "intinc/shmath.h"

bool __safe_addition(uintmax_t a, uintmax_t b, uintmax_t* place){
	if (a > (UINTMAX_MAX - b)) {
		*place = 0;
		return true;
	} else {
		*place = a+b;
		return false;
	}
}

bool __safe_multiplication(uintmax_t a, uintmax_t b, uintmax_t* place){
	if (a > UINTMAX_MAX / b) {
		*place = 0;
		return true;
	} else {
		*place = a * b;
		return false;
	}
}
