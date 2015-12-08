/*
 * locale.h
 *
 *  Created on: Dec 8, 2015
 *      Author: enerccio
 */

#ifndef LOCALE_H_
#define LOCALE_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <stddef.h>

struct lconv {
	char* decimal_point;       // "."
	char* thousands_sep;       // ""
	char* grouping;            // ""
	char* mon_decimal_point;   // ""
	char* mon_thousands_sep;   // ""
	char* mon_grouping;        // ""
	char* positive_sign;       // ""
	char* negative_sign;       // ""
	char* currency_symbol;     // ""
	char  frac_digits;          // CHAR_MAX
	char  p_cs_precedes;        // CHAR_MAX
	char  n_cs_precedes;        // CHAR_MAX
	char  p_sep_by_space;       // CHAR_MAX
	char  n_sep_by_space;       // CHAR_MAX
	char  p_sign_posn;          // CHAR_MAX
	char  n_sign_posn;          // CHAR_MAX
	char* int_curr_symbol;     // ""
	char  int_frac_digits;      // CHAR_MAX
	char  int_p_cs_precedes;    // CHAR_MAX
	char  int_n_cs_precedes;    // CHAR_MAX
	char  int_p_sep_by_space;   // CHAR_MAX
	char  int_n_sep_by_space;   // CHAR_MAX
	char  int_p_sign_posn;      // CHAR_MAX
	char  int_n_sign_posn;      // CHAR_MAX
};

#define NULL ((void*)0)
#define LC_COLLATE  ( 1<<1 )
#define LC_CTYPE    ( 1<<2 )
#define LC_MONETARY ( 1<<3 )
#define LC_NUMERIC  ( 1<<4 )
#define LC_TIME     ( 1<<5 )
#define LC_ALL 		( LC_COLLATE | LC_CTYPE | LC_MONETARY | LC_NUMERIC | LC_TIME )

#error KCLIB does not support this library yet

#ifdef __cplusplus
}
#endif

#endif /* LOCALE_H_ */
