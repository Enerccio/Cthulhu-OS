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

#define __noreturn void

extern __noreturn _kclib_assert(unsigned int lineno, const char* file, const char* func);

#endif
#ifdef __cplusplus
}
#endif

#endif /* SYS_EXTERNAL_H_ */
