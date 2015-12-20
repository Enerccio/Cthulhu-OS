/*
 * stdlib.h
 *
 *  Created on: Dec 18, 2015
 *      Author: enerccio
 */

#ifndef INTINC_STDLIB_H_
#define INTINC_STDLIB_H_

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Returns radix value for character (0 -> '0', 15 -> 'F', 16 -> 'G' etc
 */
uint8_t __radixval(char c);

#ifdef __cplusplus
}
#endif

#endif /* INTINC_STDLIB_H_ */
