/*
 * assert.h
 *
 *  Created on: Dec 8, 2015
 *      Author: enerccio
 */

#ifndef ASSERT_H_
#define ASSERT_H_

#ifdef __CDT_INDEXER__
#include <gcc_defs.h>
#endif

#ifdef __cplusplus
extern "C" {
#endif

#include <sys/external.h>
#ifndef __KCLIB_KERNEL_MODE
#include <stdio.h>
#endif

#define __ASSERT_INTERNAL(expression) \
	do { \
		if ((expression) == 0) { \
			__kclib_assert_failure_k(__LINE__, __FILE__, __func__); \
		} \
	} while (0)

#undef assert
#ifdef NDEBUG
#define assert(ignore) ((void)0)
#else
#ifdef __KCLIB_KERNEL_MODE
#define assert(expression) __ASSERT_INTERNAL(expression)
#else

// TODO

#endif
#endif


#ifdef __cplusplus
}
#endif

#endif /* ASSERT_H_ */
