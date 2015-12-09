/*
 * types.h
 *
 *  Created on: Dec 9, 2015
 *      Author: enerccio
 */

#ifndef MACHINE_TYPES_H_
#define MACHINE_TYPES_H_

#include <machine/defaults.h>

#ifdef __cplusplus
extern "C" {
#endif

#if __GNUC_PREREQ (3, 3)
/* GCC >= 3.3.0 has __<val>__ implicitly defined. */
#define __SPLIT(x) __##x##__
#else
/* Fall back to POSIX versions from <limits.h> */
#define __SPLIT(x) x
#include <limits.h>
#endif

#ifdef __cplusplus
}
#endif

#endif /* MACHINE_TYPES_H_ */
