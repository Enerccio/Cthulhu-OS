/*
 * types.h
 *
 *  Created on: Dec 10, 2015
 *      Author: enerccio
 */

#ifndef SYS_TYPES_H_
#define SYS_TYPES_H_

#ifdef __CDT_INDEXER__
#include <gcc_defs.h>
#endif

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>

typedef intmax_t blkcnt_t; // Used for file block counts
typedef intmax_t blksize_t; // Used for block sizes
typedef intmax_t clock_t; //  Used for system times in clock ticks or CLOCKS_PER_SEC (see <time.h>).
typedef intmax_t clockid_t; //  Used for clock ID type in the clock and timer functions.
typedef intmax_t dev_t;  //  Used for device IDs.
typedef uintmax_t fsblkcnt_t; //  Used for file system block counts
typedef uintmax_t fsfilcnt_t; //  Used for file system file counts
typedef intmax_t gid_t; // Used for group IDs.
typedef intmax_t id_t;  // Used as a general identifier; can be used to contain at least a pid_t, uid_t or a gid_t.
typedef uintmax_t ino_t; 			// Used for file serial numbers.
// key_t;  //    Used for interprocess communication.
typedef intmax_t mode_t;  //    Used for some file attributes.
typedef intmax_t nlink_t;  //    Used for link counts.
typedef intmax_t off_t;  //    Used for file sizes.
typedef intmax_t pid_t;  //    Used for process IDs and process group IDs.
// pthread_attr_t;  //    Used to identify a thread attribute object.
// pthread_cond_t;  //    Used for condition variables.
// pthread_condattr_t;  //    Used to identify a condition attribute object.
// pthread_key_t;  //    Used for thread-specific data keys.
// pthread_mutex_t;  //    Used for mutexes.
// pthread_mutexattr_t;  //    Used to identify a mutex attribute object.
// pthread_once_t;  //    Used for dynamic package initialisation.
// pthread_rwlock_t;  //    Used for read-write locks.
// pthread_rwlockattr_t;  //    Used for read-write lock attributes.
// pthread_t;  //    Used to identify a thread.
#ifndef __SIZE_TYPE__
typedef uintmax_t size_t;  //    Used for sizes of objects.
#endif
typedef intmax_t ssize_t;  //    Used for a count of bytes or an error indication.
typedef int32_t suseconds_t;  //    Used for time in microseconds
typedef intmax_t time_t;  //    Used for time in seconds.
typedef intmax_t timer_t;  //    Used for timer ID returned by timer_create().
typedef intmax_t uid_t;  //    Used for user IDs.
typedef uint32_t useconds_t;  //    Used for time in microseconds.

#ifndef SSIZE_MAX
#define SSIZE_MAX INTMAX_MAX
#endif

#ifdef __cplusplus
}
#endif

#endif /* SYS_TYPES_H_ */
