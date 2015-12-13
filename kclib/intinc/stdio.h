/*
 * stdio.h
 *
 *  Created on: Dec 12, 2015
 *      Author: enerccio
 */

#ifndef INTINC_STDIO_H_
#define INTINC_STDIO_H_

#ifdef __cplusplus
extern "C" {
#endif

#define __STDOUT 1
#define __STDERR 2
#define __STDIN  3

struct FILE {
	void* handle;
};

void __initialize_streams();

#ifdef __cplusplus
}
#endif

#endif /* INTINC_STDIO_H_ */
