/*
 * limits.h
 *
 *Created on: Dec 8, 2015
 *Author: enerccio
 */

#ifndef LIMITS_H_
#define LIMITS_H_

#ifdef __CDT_INDEXER__
#include <gcc_defs.h>
#endif

#ifdef __cplusplus
extern "C" {
#endif

#ifdef _MB_LEN_MAX
#define MB_LEN_MAX	_MB_LEN_MAX
#else
#define MB_LEN_MAX1
#endif

#ifndef NL_ARGMAX
#define NL_ARGMAX 32
#endif

#if !defined __GNUC__ || __GNUC__ < 2
#ifndef _LIMITS_H
#define _LIMITS_H	1

#include <machine/defaults.h>

#undef CHAR_BIT
#define CHAR_BIT 8
#undef SCHAR_MIN
#define SCHAR_MIN (-128)
#undef SCHAR_MAX
#define SCHAR_MAX 127
#undef UCHAR_MAX
#define UCHAR_MAX 255
#ifdef __CHAR_UNSIGNED__
#undef CHAR_MIN
#define CHAR_MIN 0
#undef CHAR_MAX
#define CHAR_MAX 255
#else
#undef CHAR_MIN
#define CHAR_MIN (-128)
#undef CHAR_MAX
#define CHAR_MAX 127
#endif
#undef SHRT_MIN
#define SHRT_MIN (-32767-1)
#undef SHRT_MAX
#define SHRT_MAX 32767
#undef USHRT_MAX
#define USHRT_MAX 65535
#ifndef __INT_MAX__
#define __INT_MAX__ 2147483647
#endif
#undef INT_MIN
#define INT_MIN (-INT_MAX-1)
#undef INT_MAX
#define INT_MAX __INT_MAX__
#undef UINT_MAX
#define UINT_MAX (INT_MAX * 2U + 1)
#ifndef __LONG_MAX__
#if defined (__alpha__) || (defined (__sparc__) && defined(__arch64__)) || defined (__sparcv9)
#define __LONG_MAX__ 9223372036854775807L
#else
#define __LONG_MAX__ 2147483647L
#endif
#endif
#undef LONG_MIN
#define LONG_MIN (-LONG_MAX-1)
#undef LONG_MAX
#define LONG_MAX __LONG_MAX__
#undef ULONG_MAX
#define ULONG_MAX (LONG_MAX * 2UL + 1)
#ifndef __LONG_LONG_MAX__
#define __LONG_LONG_MAX__ 9223372036854775807LL
#endif
#if (defined (__STDC_VERSION__) && __STDC_VERSION__ >= 199901L) ||\
(defined(__cplusplus) && __cplusplus >= 201103L)
#undef LLONG_MIN
#define LLONG_MIN (-LLONG_MAX-1)
#undef LLONG_MAX
#define LLONG_MAX __LONG_LONG_MAX__
#undef ULLONG_MAX
#define ULLONG_MAX (LLONG_MAX * 2ULL + 1)
#endif
#if defined (__GNU_LIBRARY__) ? defined (__USE_GNU) : !defined (__STRICT_ANSI__)
#undef LONG_LONG_MIN
#define LONG_LONG_MIN (-LONG_LONG_MAX-1)
#undef LONG_LONG_MAX
#define LONG_LONG_MAX __LONG_LONG_MAX__
#undef ULONG_LONG_MAX
#define ULONG_LONG_MAX (LONG_LONG_MAX * 2ULL + 1)
#endif
#endif
#endif
#endif

#ifdef __cplusplus
}
#endif

#if defined __GNUC__ && !defined _GCC_LIMITS_H_
#include_next <limits.h>
#endif

#ifndef _POSIX2_RE_DUP_MAX
#define _POSIX2_RE_DUP_MAX 255
#endif

#ifndef ARG_MAX
#define ARG_MAX 4096
#endif

#ifndef PATH_MAX
#define PATH_MAX 4096
#endif
