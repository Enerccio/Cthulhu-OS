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
/**
 * Same as memcpy, but stores true into flagged if flag_char is encountered
 *
 * - slower than memcpy
 */
void* __memcpy_cb(void* restrict s1, const void* restrict s2, size_t n, bool* flagged, char flag_char);
/**
 * Same as memmove, but stores true into flagged if flag_char is encountered
 *
 * - slower than memmove
 */
void* __memmove_cb(void* s1, const void* s2, size_t n, bool* flagged, char flag_char);


#ifdef __cplusplus
}
#endif

#endif /* INTINC_STRING_H_ */
