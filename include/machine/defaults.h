/*
 * defaults.h
 *
 *  Created on: Dec 9, 2015
 *      Author: enerccio
 */

#ifndef MACHINE_DEFAULTS_H_
#define MACHINE_DEFAULTS_H_

#ifdef __CDT_INDEXER__
#include <gcc_defs.h>
#endif

#ifdef __cplusplus
extern "C" {
#endif

#ifndef __GNUC_PREREQ
# if defined __GNUC__ && defined __GNUC_MINOR__
#  define __GNUC_PREREQ(maj, min) \
	((__GNUC__ << 16) + __GNUC_MINOR__ >= ((maj) << 16) + (min))
# else
#  define __GNUC_PREREQ(maj, min) 0
# endif
#endif

#ifndef __IEEE_BIG_ENDIAN
#ifndef __IEEE_LITTLE_ENDIAN

#ifdef __x86_64__
#define __IEEE_LITTLE_ENDIAN
#endif

#ifdef _WIN32
#define __IEEE_LITTLE_ENDIAN
#endif

#ifdef __i386__
#define __IEEE_LITTLE_ENDIAN
#endif

#ifndef __IEEE_BIG_ENDIAN
#ifndef __IEEE_LITTLE_ENDIAN
#error Unable to query endianess for this system
#endif /* not __IEEE_LITTLE_ENDIAN */
#endif /* not __IEEE_BIG_ENDIAN */

#endif
#endif

#ifdef __cplusplus
}
#endif

#endif /* MACHINE_DEFAULTS_H_ */
