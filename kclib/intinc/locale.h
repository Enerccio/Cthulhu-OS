/*
 * locale.h
 *
 *  Created on: Dec 11, 2015
 *      Author: enerccio
 */

#ifndef INTINC_LOCALE_H_
#define INTINC_LOCALE_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <stddef.h>
#include <stdint.h>

/**
 * Initializes locale, internal call
 */
void __initialize_locale();

/**
 * Single collating character
 *
 * contains character sequence corresponding to collating character and
 * assigned weights
 */
typedef struct lcoll_elem {
	uint8_t* elem;
	size_t nelems;

	int32_t* weights;
	size_t nweights;
} lcoll_elem_t;

/**
 * Collation table locale
 *
 * contains collating elemens + default weights and their count
 */
typedef struct lcoll {
	lcoll_elem_t** colls;
	size_t nelems;

	int32_t* defe_weights;
	size_t defe_wcount;
} lcoll_t;

/**
 * Character table locale
 */
typedef struct lctype {
	uint8_t* space_table;
	uint8_t* blank_table;
	uint8_t* lower_table;
	uint8_t* upper_table;
	uint8_t* digit_table;
	uint8_t* xdigit_table;
	uint8_t* graph_table;
} lctype_h;

/**
 * Collatelly compares next "character" in the streams provided, moving them as appropriate
 */
int __compare_collate(char** a, char** b);

/**
 * Returns ctype table from current locale
 */
lctype_h* __getlctype();

#ifdef __cplusplus
}
#endif

#endif /* INTINC_LOCALE_H_ */
