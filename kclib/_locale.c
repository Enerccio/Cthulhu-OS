#include <locale.h>
#include <string.h>
#include <limits.h>
#include "intinc/locale.h"

typedef struct locale {
	char* lc_ctype;
	char* lc_collate;
	char* lc_monetary;
	char* lc_numeric;
	char* lc_time;

	struct lconv conversion_locale;
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
