/*
 * kernel.h
 *
 *  Created on: Dec 11, 2015
 *      Author: enerccio
 */

#ifndef SYS_KERNEL_H_
#define SYS_KERNEL_H_

#ifdef __CDT_INDEXER__
#include <gcc_defs.h>
#endif

#ifdef __cplusplus
extern "C" {
#endif

/* This needs to be called by kernel before using functions from this libc.
 * Calling any function before calling this function results in
 * undefined behavior. */

#ifdef __KCLIB_KERNEL_MODE
void __initialize_kclib();
#endif

#ifdef __cplusplus
}
#endif

#endif /* SYS_KERNEL_H_ */
