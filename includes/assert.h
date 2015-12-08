/*
 * assert.h
 *
 *  Created on: Dec 8, 2015
 *      Author: enerccio
 */

#ifndef ASSERT_H_
#define ASSERT_H_

#include "_kclib_commons.h"
#include <stdio.h>

#ifdef __cplusplus
extern "C" {
#endif

#undef assert
#ifdef NDEBUG
#define assert(ignore) ((void)0)
#else
#error TODO

#define assert(expression) \
	do { \
		if (expression == 0) { \
		} \
	} while (0)
#endif


#ifdef __cplusplus
}
#endif

#endif /* ASSERT_H_ */
