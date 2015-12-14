#ifndef INTINC_SHMATH_H_
#define INTINC_SHMATH_H_

#include <stdbool.h>
#include <stdint.h>
#include <limits.h>

#if __GNUC__ >= 5
#define __WILL_OVERFLOW_ADD(a, b, where) __builtin_add_overflow(a, b, where)
#define __WILL_OVERFLOW_MUL(a, b, where) __builtin_mul_overflow(a, b, where)
#else
bool __safe_addition(uintmax_t a, uintmax_t b, uintmax_t* place);
#define __WILL_OVERFLOW_ADD(a, b, where) __safe_addition(a, b, where)
bool __safe_multiplication(uintmax_t a, uintmax_t b, uintmax_t* place);
#define __WILL_OVERFLOW_MUL(a, b, where) __safe_multiplication(a, b, where)
#endif

#endif /* INTINC_SHMATH_H_ */
