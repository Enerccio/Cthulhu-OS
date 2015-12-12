#include <locale.h>
#include <string.h>
#include <limits.h>
#include <stdlib.h>
#include "intinc/locale.h"

typedef struct locale {
	char* lc_ctype;
	char* lc_collate;
	char* lc_monetary;
	char* lc_numeric;
	char* lc_time;

	struct lconv conversion_locale;
	lcoll_t collation_locale;
} locale_t;

#ifndef __DEFAULT_LOCALE
#define __DEFAULT_LOCALE "C"
#endif

#define __SUPPORTED_LOCALE_COUNT 1

static locale_t active_locale;
locale_t locales[__SUPPORTED_LOCALE_COUNT];
char* localenames[__SUPPORTED_LOCALE_COUNT];

struct lconv* localeconv(){
	static struct lconv conv;
	memmove(&active_locale.conversion_locale, &conv, sizeof(struct lconv));
	return &conv;
}

char* setlocale(int category, const char* locale){
	char* ret = NULL;

	switch (category){
	case LC_COLLATE: ret = active_locale.lc_collate; break;
	case LC_CTYPE: ret = active_locale.lc_ctype; break;
	case LC_MONETARY: ret = active_locale.lc_monetary; break;
	case LC_NUMERIC: ret = active_locale.lc_numeric; break;
	case LC_TIME : ret = active_locale.lc_time; break;
	default:
	case LC_ALL: ret = "<invalid>";
	}

	if (ret == NULL)
		ret = "<invalid>";

	if (locale == NULL){
		return ret;
	}

	locale_t* newlocale;
	char* locale_name = strtok((char*)locale, ":");
	if (locale_name == NULL)
		locale_name = (char*)locale;

	for (uint16_t loc_id = 0; loc_id < __SUPPORTED_LOCALE_COUNT; loc_id++){
		if (strcmp(locale_name, localenames[loc_id]) == 0){
			newlocale = &locales[loc_id];
		}
	}

	if (newlocale == NULL){
		return NULL;
	}

	if ((category & LC_COLLATE) == LC_COLLATE){
		active_locale.lc_collate = newlocale->lc_collate;
		active_locale.collation_locale = newlocale->collation_locale;
	}
	if ((category & LC_CTYPE) == LC_CTYPE){

	}
	if ((category & LC_MONETARY) == LC_MONETARY){
		active_locale.lc_monetary = newlocale->lc_monetary;
		active_locale.conversion_locale.int_curr_symbol = newlocale->conversion_locale.int_curr_symbol;
		active_locale.conversion_locale.currency_symbol = newlocale->conversion_locale.currency_symbol;
		active_locale.conversion_locale.mon_decimal_point = newlocale->conversion_locale.mon_decimal_point;
		active_locale.conversion_locale.mon_thousands_sep = newlocale->conversion_locale.mon_thousands_sep;
		active_locale.conversion_locale.mon_grouping = newlocale->conversion_locale.mon_grouping;
		active_locale.conversion_locale.positive_sign = newlocale->conversion_locale.positive_sign;
		active_locale.conversion_locale.negative_sign = newlocale->conversion_locale.negative_sign;
		active_locale.conversion_locale.int_frac_digits = newlocale->conversion_locale.int_frac_digits;
		active_locale.conversion_locale.frac_digits = newlocale->conversion_locale.frac_digits;
		active_locale.conversion_locale.p_cs_precedes = newlocale->conversion_locale.p_cs_precedes;
		active_locale.conversion_locale.p_sep_by_space = newlocale->conversion_locale.p_sep_by_space;
		active_locale.conversion_locale.n_cs_precedes = newlocale->conversion_locale.n_cs_precedes;
		active_locale.conversion_locale.n_sep_by_space = newlocale->conversion_locale.n_sep_by_space;
		active_locale.conversion_locale.p_sign_posn = newlocale->conversion_locale.p_sign_posn;
		active_locale.conversion_locale.n_sign_posn = newlocale->conversion_locale.n_sign_posn;
	}
	if ((category & LC_NUMERIC) == LC_NUMERIC){
		active_locale.lc_numeric = newlocale->lc_numeric;
		active_locale.conversion_locale.decimal_point = newlocale->conversion_locale.decimal_point;
		active_locale.conversion_locale.thousands_sep = newlocale->conversion_locale.thousands_sep;
		active_locale.conversion_locale.grouping = newlocale->conversion_locale.grouping;
	}
	if ((category & LC_TIME) == LC_TIME){

	}

	return ret;
}

void __generate_stdasciicollate(lcoll_t* collate){
	collate->nelems = 127;
	collate->colls = malloc(sizeof(lcoll_elem_t*)*collate->nelems);
	uint8_t* colids = malloc(sizeof(uint8_t)*collate->nelems);
	int32_t* weights = malloc(sizeof(size_t)*collate->nelems+1);
	for (uint8_t i=0; i<collate->nelems; i++){
		lcoll_elem_t* colelem = malloc(sizeof(lcoll_elem_t));
		colelem->elem = &colids[i];
		colelem->weights = &weights[i];

		colelem->nelems = 1;
		colelem->nweights = 1;

		colids[i] = i;
		weights[i] = i;
		collate->colls[i] = colelem;
	}
	collate->defe_wcount = 1;
	collate->defe_weights = &weights[collate->nelems];
	weights[collate->nelems] = collate->nelems;
}

void __initialize_locale(){
	localenames[0] = "C";
	locale_t* std_locale = &locales[0];

	struct lconv def_loc_conv = {
		".", "", "", "", "", "", "", "", "",
		CHAR_MAX, CHAR_MAX, CHAR_MAX, CHAR_MAX,
		CHAR_MAX, CHAR_MAX, CHAR_MAX, "",
		CHAR_MAX, CHAR_MAX, CHAR_MAX, CHAR_MAX,
		CHAR_MAX, CHAR_MAX, CHAR_MAX
	};

	memmove(&std_locale->conversion_locale, &def_loc_conv, sizeof(struct lconv));
	__generate_stdasciicollate(&std_locale->collation_locale);

	std_locale->lc_collate = "C:LC_COLLATE";
	std_locale->lc_ctype = "C:LC_CTYPE";
	std_locale->lc_monetary = "C:LC_MONETARY";
	std_locale->lc_numeric = "C:LC_NUMERIC";
	std_locale->lc_time = "C:LC_TIME";

	active_locale.lc_ctype = NULL;
	active_locale.lc_collate = NULL;
	active_locale.lc_monetary = NULL;
	active_locale.lc_numeric = NULL;
	active_locale.lc_time = NULL;

	setlocale(LC_ALL, "C");
}

size_t __find_weights(char** stream, int32_t** weights){
	lcoll_t* ctable = &active_locale.collation_locale;

	for (size_t i=0; i<ctable->nelems; i++){
		lcoll_elem_t* element = ctable->colls[i];
		char* b = *stream;

		for (size_t j=0; j<element->nelems; j++){
			if (*(b++) != element->elem[j])
				goto outer;
		}

		*stream = b;
		*weights = element->weights;
		return element->nweights;

		outer:;
	}

	*weights = ctable->defe_weights;

	return ctable->defe_wcount;
}

int __compare_collate(char** a, char** b){
	int32_t* wa;
	size_t wac;
	int32_t* wb;
	size_t wbc;

	wac = __find_weights(a, &wa);
	wbc = __find_weights(b, &wb);

repeat_search:;
	size_t scount = wac > wbc ? wbc : wac;

	for (size_t i=0; i<scount; i++){
		int32_t weight_a = wa[i];
		int32_t weight_b = wb[i];
		if (wa[i] == -1 || wb[i] == -1){
			if (wa[i] == -1)
				wac = __find_weights(a, &wa);
			if (wb[i] == -1)
				wbc = __find_weights(b, &wb);
			goto repeat_search;
		}

		if (weight_a < weight_b)
			return -1;
		if (weight_a > weight_b)
			return 1;
	}

	return 0;
}
