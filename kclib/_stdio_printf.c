#include <stdio.h>
#include <errno.h>
#include "intinc/stdio.h"

int printf(const char* restrict format, ...){
	va_list args;
	va_start(args, format);
	int retval = vfprintf(stdout, format, args);
	va_end(args);
	return retval;
}

int fprintf(FILE* restrict stream,
		const char* restrict format, ...){
	va_list args;
	va_start(args, format);
	int retval = vfprintf(stream, format, args);
	va_end(args);
	return retval;
}

#define __CHECK_INVALID(format) \
	do { if (*format == '\0') return __FORMAT_ERROR_NOT_ENOUGH_DATA; } while (0)

#define __PRINTF_JUSTIFY (1<<1)
#define __PRINTF_JUSTIFY_LEFT(flags) ((flags & __PRINTF_JUSTIFY) == __PRINTF_JUSTIFY)
#define __PRINTF_JUSTIFY_RIGHT(flags) ((flags & __PRINTF_JUSTIFY) == 0)
#define __PRINTF_S_JUSTIFY_LEFT(flags) do {*flags |= __PRINTF_JUSTIFY; } while (0)
#define __PRINTF_S_JUSTIFY_RIGHT(flags) do {*flags ^= __PRINTF_JUSTIFY; } while (0)

#define __PRINTF_SIGNCONV (1<<2)
#define __PRINTF_SIGNCONV_ALLWAYS(flags) ((flags & __PRINTF_SIGNCONV) == __PRINTF_SIGNCONV)
#define __PRINTF_SIGNCONV_NEGATIVE(flags) ((flags & __PRINTF_SIGNCONV) == 0)
#define __PRINTF_S_SIGNCONV_ALLWAYS(flags) do {*flags |= __PRINTF_SIGNCONV; } while (0)
#define __PRINTF_S_SIGNCONV_NEGATIVE(flags) do {*flags ^= __PRINTF_SIGNCONV; } while (0)

#define __PRINTF_SIGNCONV_SPACE (1<<3)
#define __PRINTF_SIGNCONV_SPACE_ALLWAYS(flags) ((flags & __PRINTF_SIGNCONV_SPACE) == __PRINTF_SIGNCONV_SPACE)
#define __PRINTF_SIGNCONV_SPACE_NEGATIVE(flags) ((flags & __PRINTF_SIGNCONV_SPACE) == 0)
#define __PRINTF_S_SIGNCONV_SPACE_ALLWAYS(flags) do {*flags |= __PRINTF_SIGNCONV_SPACE; } while (0)
#define __PRINTF_S_SIGNCONV_SPACE_NEGATIVE(flags) do {*flags ^= __PRINTF_SIGNCONV_SPACE; } while (0)

#define __PRINTF_ALTERF (1<<4)
#define __PRINTF_ALTERF_YES(flags) ((flags & __PRINTF_ALTERF) == __PRINTF_ALTERF)
#define __PRINTF_ALTERF_NO(flags) ((flags & __PRINTF_ALTERF) == 0)
#define __PRINTF_S_ALTERF_YES(flags) do {*flags |= __PRINTF_ALTERF; } while (0)
#define __PRINTF_S_ALTERF_NO(flags) do {*flags ^= __PRINTF_ALTERF; } while (0)

int __printf_flags(char** format, uint64_t* flags){
	switch (**format){
	case '-': __PRINTF_S_JUSTIFY_LEFT(flags); break;
	case '+': __PRINTF_S_SIGNCONV_ALLWAYS(flags); break;
	case ' ': __PRINTF_S_SIGNCONV_SPACE_ALLWAYS(flags); break;
	case '#': __PRINTF_S_ALTERF_YES(flags); break;
	default:
		return 1;
	}
	++*format;
	return 0;
}

#define __PRINTF_CHECKMOD(format, target, spec, type) \
	if (strncmp(format, spec, sizeof(spec)) == 0) { target = type; format += sizeof(spec); }
#define __PRINTF_LENMOD_HH 1
#define __PRINTF_LENMOD_HH_TEST(format, target) __PRINTF_CHECKMOD(format, target, "hh", __PRINTF_LENMOD_HH)
#define __PRINTF_LENMOD_H 2
#define __PRINTF_LENMOD_H_TEST(format, target) __PRINTF_CHECKMOD(format, target, "h", __PRINTF_LENMOD_H)
#define __PRINTF_LENMOD_L 3
#define __PRINTF_LENMOD_L_TEST(format, target) __PRINTF_CHECKMOD(format, target, "l", __PRINTF_LENMOD_L)
#define __PRINTF_LENMOD_LL 4
#define __PRINTF_LENMOD_LL_TEST(format, target) __PRINTF_CHECKMOD(format, target, "ll", __PRINTF_LENMOD_LL)
#define __PRINTF_LENMOD_J 5
#define __PRINTF_LENMOD_J_TEST(format, target) __PRINTF_CHECKMOD(format, target, "j", __PRINTF_LENMOD_J)
#define __PRINTF_LENMOD_Z 6
#define __PRINTF_LENMOD_Z_TEST(format, target) __PRINTF_CHECKMOD(format, target, "z", __PRINTF_LENMOD_HH)
#define __PRINTF_LENMOD_T 7
#define __PRINTF_LENMOD_T_TEST(format, target) __PRINTF_CHECKMOD(format, target, "t", __PRINTF_LENMOD_T)
#define __PRINTF_LENMOD_BIGL 8
#define __PRINTF_LENMOD_BIGL_TEST(format, target) __PRINTF_CHECKMOD(format, target, "L", __PRINTF_LENMOD_BIGL)

#define __PRINTF_CONV_INT_TO_SINT 0
#define __PRINTF_CONV_UINT_TO_UOCT 1
#define __PRINTF_CONV_UINT_TO_UDEC 2
#define __PRINTF_CONV_UINT_TO_UHEX_L 3
#define __PRINTF_CONV_UINT_TO_UHEX_U 4
#define __PRINTF_CONV_DBL_TO_FLT_DECNOT 5
#define __PRINTF_CONV_DBL_TO_FLT_DECNOT_U 55
#define __PRINTF_CONV_DBL_TO_FLT_ENOT 6
#define __PRINTF_CONV_DBL_TO_FLT_ENOT_U 66
#define __PRINTF_CONV_DBL_TO_FLT_DECORENOT 7
#define __PRINTF_CONV_DBL_TO_FLT_DECORENOT_U 77
#define __PRINTF_CONV_DBL_TO_FLT_HEXNOT 8
#define __PRINTF_CONV_DBL_TO_FLT_HEXNOT_U 88
#define __PRINTF_CONV_CHAR 9
#define __PRINTF_CONV_STRING 10
#define __PRINTF_CONV_PTR 11
#define __PRINTF_CONV_WRITETO 12
#define __PRINTF_CONV_PERC 13

// TODO: Use ferror
#define __PRINTF_WRITE(source, target, size) \
	do { \
		size_t wa = fwrite(source, size, 1, target); \
		if (wa!=size || target->error != 0) \
			return EOF; \
	} while (0)

int __vprintf_arg(FILE* stream, char** fplace, size_t* writelen,
					uint64_t flags, int widthspec, uint64_t lenmod, uint64_t type, va_list args){
	if (type == __PRINTF_CONV_PERC){
		__PRINTF_WRITE("%", stream, 1);
		++*writelen;
		return 0;
	}

	if (type == __PRINTF_CONV_WRITETO){
		int* ptr = va_arg(args, int*);
		*ptr = *writelen;
		return 0;
	}

	if (type == __PRINTF_CONV_STRING){
		// TODO: handle l modifier / wchar_t

		char* ptr = va_arg(args, char*);
		if (widthspec == -1){
			widthspec = strlen(ptr);
		}

		__PRINTF_WRITE(ptr, stream, (unsigned int)widthspec);

		*writelen += widthspec;
	}

	// TODO rest
	return 0;
}

int vfprintf(FILE* restrict stream,
		const char* restrict format,
		va_list arg){

	char* fmt = (char*) format;
	size_t written_amount = 0;

	while (*fmt != '\0'){
		size_t std_segment_size = strcspn(fmt, "%");

		__PRINTF_WRITE(fmt, stream, std_segment_size);
		fmt += std_segment_size;
		written_amount += std_segment_size;

		if (*fmt != '\0'){
			// we have % modifier, deal with it
			++fmt;
			__CHECK_INVALID(fmt);

			uint64_t flags = 0;

			do {__CHECK_INVALID(fmt);} while (__printf_flags(&fmt, &flags));

			__CHECK_INVALID(fmt);

			int widthspec = -1;
			if (*fmt == '*') {
				widthspec = va_arg(arg, int);
				++fmt;
			} else {
				widthspec = (int)strtol((const char*)fmt, &fmt, 10);
				if (errno == ERANGE){
					errno = 0;
					return __FORMAT_ERROR_LENSPEC_INCORRECT;
				}
			}

			__CHECK_INVALID(fmt);

			uint64_t lenmod = 0;

			__PRINTF_LENMOD_HH_TEST(fmt, lenmod)
			else __PRINTF_LENMOD_H_TEST(fmt, lenmod)
			else __PRINTF_LENMOD_L_TEST(fmt, lenmod)
			else __PRINTF_LENMOD_LL_TEST(fmt, lenmod)
			else __PRINTF_LENMOD_J_TEST(fmt, lenmod)
			else __PRINTF_LENMOD_Z_TEST(fmt, lenmod)
			else __PRINTF_LENMOD_T_TEST(fmt, lenmod)
			else __PRINTF_LENMOD_BIGL_TEST(fmt, lenmod)

			__CHECK_INVALID(fmt);

			char typespec = *fmt;
			uint64_t type;

			switch (typespec){
			case 'd':
			case 'i':
				type = __PRINTF_CONV_INT_TO_SINT;
				break;
			case 'o':
				type = __PRINTF_CONV_UINT_TO_UOCT;
				break;
			case 'u':
				type = __PRINTF_CONV_UINT_TO_UDEC;
				break;
			case 'x':
				type = __PRINTF_CONV_UINT_TO_UHEX_L;
				break;
			case 'X':
				type = __PRINTF_CONV_UINT_TO_UHEX_U;
				break;
			case 'f':
				type = __PRINTF_CONV_DBL_TO_FLT_DECNOT;
				break;
			case 'F':
				type = __PRINTF_CONV_DBL_TO_FLT_DECNOT_U;
				break;
			case 'e':
				type = __PRINTF_CONV_DBL_TO_FLT_DECNOT;
				break;
			case 'E':
				type = __PRINTF_CONV_DBL_TO_FLT_DECNOT_U;
				break;
			case 'g':
				type = __PRINTF_CONV_DBL_TO_FLT_DECORENOT;
				break;
			case 'G':
				type = __PRINTF_CONV_DBL_TO_FLT_DECORENOT_U;
				break;
			case 'a':
				type = __PRINTF_CONV_DBL_TO_FLT_HEXNOT;
				break;
			case 'A':
				type = __PRINTF_CONV_DBL_TO_FLT_HEXNOT_U;
				break;
			case 'c':
				type = __PRINTF_CONV_CHAR;
				break;
			case 's':
				type = __PRINTF_CONV_STRING;
				break;
			case 'p':
				type = __PRINTF_CONV_PTR;
				break;
			case 'n':
				type = __PRINTF_CONV_WRITETO;
				break;
			case '%':
				type = __PRINTF_CONV_PERC;
				break;
			default:
				return __FORMAT_ERROR_WRONG_SPECS;
			}
			++fmt;

			int retval = __vprintf_arg(stream, &fmt, &written_amount, flags, widthspec, lenmod, type, arg);
			if (retval)
				return retval;
		}
	}

	return 0;
}
