/*
 * ctype.h
 *
 *  Created on: Dec 8, 2015
 *      Author: enerccio
 */

#ifndef CTYPE_H_
#define CTYPE_H_

#ifdef __CDT_INDEXER__
#include <gcc_defs.h>
#endif

#ifdef __cplusplus
extern "C" {
#endif

int isalnum(int c);
int isalpha(int c);
int isblank(int c);
int iscntrl(int c);
int isdigit(int c);
int isgraph(int c);
int islower(int c);
int isprint(int c);
int ispunct(int c);
int isspace(int c);
int isupper(int c);
int isxdigit(int c);
int tolower(int c);
int toupper(int c);

#ifdef __cplusplus
}
#endif

#endif /* CTYPE_H_ */
