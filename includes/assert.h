/*
 * assert.h
 *
 *  Created on: Dec 8, 2015
 *      Author: enerccio
 */

#ifndef ASSERT_H_
#define ASSERT_H_

#include <stdio.h>

#ifdef __cplusplus
extern "C" {
#endif

#undef assert
#ifdef NDEBUG
#define assert(ignore) ((void)0)
#else
#include <sys/external.h>
#ifdef KCLIB_KERNEL_MODE
#define assert(expression) \
	do { \
		if (expression == 0) { \
			_kclib_assert(__LINE__, __FILE__, __func__); \
		} \
	} while (0)
#else
#error TODO
#endif
#endif


#ifdef __cplusplus
}
#endif

#endif /* ASSERT_H_ */
