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
#ifdef __KCLIB_KERNEL_MODE

#include <stdint.h>

#ifndef __ALLOC_PAGE_SIZE
#define __ALLOC_PAGE_SIZE 0x1000
#endif

#define __noreturn void

/**
 * Called when libc has detected unrecoverable error. Only called in
 * kernel mode, in user mode abort() is called instead. What kernel
 * should do is up to the implementer. However, any calls to kclib
 * functions are now undefined behavior.
 */
extern __noreturn __kclib_assert_failure_k(uint32_t lineno, const char* file, const char* func);

extern void __kclib_stdputc_k(char c);

#endif

/**
 * Returns kernel heap start for kernel space, or returns start of the heap
 * section in user space.
 */
extern void* 	  __kclib_heap_start();
/**
 * Allocates the physical memory to that virtual address or syscall for more
 * memory from kernel.
 */
extern void*	  __kclib_allocate(uintptr_t afrom, size_t aamount);
/**
 * Deallocate the physical memory in kernel space or marks memory as unused
 * in user space
 */
extern void 	  __kclib_deallocate(uintptr_t afrom, size_t aamount);
/***
 * Should return 1 if address is reclaimed by kernel in kernel space or
 * not used in user space, so malloc library can modify it's highest used
 * address
 */
extern uint8_t    __kclib_isreclaimed(uintptr_t afrom, size_t aamount);

#ifdef __cplusplus
}
#endif

#endif /* SYS_EXTERNAL_H_ */
