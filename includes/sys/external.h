/*
 * external.h
 *
 *  Created on: Dec 8, 2015
 *      Author: enerccio
 */

#ifndef SYS_EXTERNAL_H_
#define SYS_EXTERNAL_H_

#ifdef __cplusplus
extern "C" {
#endif
#ifdef KCLIB_KERNEL_MODE

#include <stdint.h>

#ifndef __ALLOC_PAGE_SIZE
#define __ALLOC_PAGE_SIZE 0x1000
#endif

#define __noreturn void

extern __noreturn _kclib_assert(uint32_t lineno, const char* file, const char* func);

#endif

extern void* 	  _kclib_heap_start();
extern void*	  _kclib_allocate(uint64_t afrom, size_t aamount);
extern void 	  _kclib_deallocate(uint64_t afrom, size_t aamount);

#ifdef __cplusplus
}
#endif

#endif /* SYS_EXTERNAL_H_ */
