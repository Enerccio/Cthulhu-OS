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

#include <stdint.h>

void __initialize_locale();

typedef struct lcoll_elem {
	uint8_t* elem;
	size_t nelems;

	int32_t* weights;
	size_t nweights;
} lcoll_elem_t;

typedef struct lcoll {
	lcoll_elem_t** colls;
	size_t nelems;

	int32_t* defe_weights;
	size_t defe_wcount;
} lcoll_t;

int __compare_collate(char** a, char** b);

#ifdef __cplusplus
}
#endif

#endif /* INTINC_LOCALE_H_ */
