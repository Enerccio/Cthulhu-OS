/*
 * errno.h
 *
 *  Created on: Dec 8, 2015
 *      Author: enerccio
 */

#ifndef ERRNO_H_
#define ERRNO_H_

#ifdef __cplusplus
extern "C" {
#endif

#define EDEFAULTERR 0
#define EDOM		1
#define EILSEQ		2
#define ERANGE		3

extern volatile int errno;


#ifdef __cplusplus
}
#endif

#endif /* ERRNO_H_ */
