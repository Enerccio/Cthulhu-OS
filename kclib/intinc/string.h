/*
 * string.h
 *
 *  Created on: Dec 18, 2015
 *      Author: pvan
 */

#ifndef INTINC_STRING_H_
#define INTINC_STRING_H_


#ifdef __cplusplus
extern "C" {
#endif

/**
 * Thread safe call to strtok
 */
char* __strtok_ts(char** token, char* restrict s1, const char* restrict s2);

#ifdef __cplusplus
}
#endif

#endif /* INTINC_STRING_H_ */
