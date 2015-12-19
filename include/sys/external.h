/*
 * external.h
 *
 *  Created on: Dec 8, 2015
 *      Author: enerccio
 */

#ifndef SYS_EXTERNAL_H_
#define SYS_EXTERNAL_H_

#ifdef __CDT_INDEXER__
#include <gcc_defs.h>
#endif

#include <stdint.h>
#include <stdlib.h>
#include <sys/stat.h>

#ifdef __cplusplus
extern "C" {
#endif

#ifndef __ALLOC_PAGE_SIZE
#define __ALLOC_PAGE_SIZE 0x1000
#endif

#ifndef __NEWLINE
#define __NEWLINE '\n'
#endif

#ifdef __KCLIB_KERNEL_MODE

// kernel space required functions

#define __noreturn void

/**
 * Called when libc has detected unrecoverable error. Only called in
 * kernel mode, in user mode abort() is called instead. What kernel
 * should do is up to the implementer. However, any calls to kclib
 * functions are now undefined behavior.
 */
extern __noreturn __kclib_assert_failure_k(uint32_t lineno, const char* file, const char* func);

#else
// user space only required functions

/**
 * Returns value for key in environment, null if there is none
 */
extern char*	  __kclib_environ_search_u(char* key);

#define __MODE_READ   (1<<1)
#define __MODE_WRITE  (1<<2)
#define __MODE_APPEND (1<<3)
#define __MODE_BINARY (1<<4)
#define __MODE_UPDATE (1<<5)
/**
 * Opens file with mode and path
 */
extern void*	  __kclib_open_file_u(const char* path, uint32_t mode);
/**
 * Fills the stat parameter for provided descriptor
 */
extern int		  __kclib_fstat_u(void* descriptor, struct stat* stat, int* errno);

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
/**
 * Should return 1 if address is reclaimed by kernel in kernel space or
 * not used in user space, so malloc library can modify it's highest used
 * address
 */
extern uint8_t    __kclib_isreclaimed(uintptr_t afrom, size_t aamount);
/**
 * Request standard stream file descriptor
 * Sent parameters are 1 - stdin, 2 - stderr, 3 - stdin
 */
extern void*	  __kclib_open_std_stream(uint8_t request_mode);
/**
 * Sends the buffer to specified stream
 * Returns actual written data or -1 on failure
 */
extern ptrdiff_t  __kclib_send_data(void* stream, uint8_t* array, size_t buffer_size);
/**
 * Reads the data to buffer from file
 * Returns actually read data or -1 on failure
 */
extern ptrdiff_t  __kclib_read_data(void* stream, uint8_t* buffer, size_t read_amount);

#ifdef __cplusplus
}
#endif

#endif /* SYS_EXTERNAL_H_ */
