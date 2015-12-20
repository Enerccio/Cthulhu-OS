#include <stdio.h>
#include <errno.h>
#include "intinc/stdio.h"

int vprintf(const char* restrict format, va_list arg){
	return vfprintf(stdout, format, arg);
}

int vsprintf(char* restrict s, const char* restrict format, va_list arg){
	return vsnprintf(s, (size_t)-1, format, arg);
}

int vsnprintf(char* restrict s, size_t n, const char* restrict format, va_list arg){
	FILE* vfs = __create_vstream((uint8_t*)s, n, _IOLBF);
	if (vfs == NULL)
		return EOF;
	int retval = vfprintf(vfs, format, arg);
	fclose(vfs);
	free(vfs);
	return retval;
}

int snprintf(char* restrict s, size_t n, const char* restrict format, ...){
	va_list args;
	va_start(args, format);
	int retval = vsnprintf(s, n, format, args);
	va_end(args);
	return retval;
}

int sprintf(char* restrict s, const char* restrict format, ...){
	va_list args;
	va_start(args, format);
	int retval = vsprintf(s, format, args);
	va_end(args);
	return retval;
}

int printf(const char* restrict format, ...){
	va_list args;
	va_start(args, format);
	int retval = vprintf(format, args);
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

// TODO: Use ferror
#define __PRINTF_WRITE(source, target, size) \
	do { \
		size_t wa = fwrite(source, 1, size, target); \
		if (wa!=size || target->error != 0) \
			return EOF; \
	} while (0)

#define __CHECK_INVALID(format) \
	do { if (*format == '\0') return __FORMAT_ERROR_NOT_ENOUGH_DATA; } while (0)

#define __PRINTF_JUSTIFY (1<<1)
#define __PRINTF_JUSTIFY_LEFT(flags) ((flags & __PRINTF_JUSTIFY) == 0)
#define __PRINTF_JUSTIFY_RIGHT(flags) ((flags & __PRINTF_JUSTIFY) == __PRINTF_JUSTIFY)
#define __PRINTF_S_JUSTIFY_LEFT(flags) do {*flags &= __PRINTF_JUSTIFY; } while (0)
#define __PRINTF_S_JUSTIFY_RIGHT(flags) do {*flags |= ~__PRINTF_JUSTIFY; } while (0)

#define __PRINTF_SIGNCONV (1<<2)
#define __PRINTF_SIGNCONV_ALLWAYS(flags) ((flags & __PRINTF_SIGNCONV) == __PRINTF_SIGNCONV)
#define __PRINTF_SIGNCONV_NEGATIVE(flags) ((flags & __PRINTF_SIGNCONV) == 0)
#define __PRINTF_S_SIGNCONV_ALLWAYS(flags) do {*flags |= __PRINTF_SIGNCONV; } while (0)
#define __PRINTF_S_SIGNCONV_NEGATIVE(flags) do {*flags &= ~__PRINTF_SIGNCONV; } while (0)

#define __PRINTF_SIGNCONV_SPACE (1<<3)
#define __PRINTF_SIGNCONV_SPACE_ALLWAYS(flags) ((flags & __PRINTF_SIGNCONV_SPACE) == __PRINTF_SIGNCONV_SPACE)
#define __PRINTF_SIGNCONV_SPACE_NEGATIVE(flags) ((flags & __PRINTF_SIGNCONV_SPACE) == 0)
#define __PRINTF_S_SIGNCONV_SPACE_ALLWAYS(flags) do {*flags |= __PRINTF_SIGNCONV_SPACE; } while (0)
#define __PRINTF_S_SIGNCONV_SPACE_NEGATIVE(flags) do {*flags &= ~__PRINTF_SIGNCONV_SPACE; } while (0)

#define __PRINTF_ALTERF (1<<4)
#define __PRINTF_ALTERF_YES(flags) ((flags & __PRINTF_ALTERF) == __PRINTF_ALTERF)
#define __PRINTF_ALTERF_NO(flags) ((flags & __PRINTF_ALTERF) == 0)
#define __PRINTF_S_ALTERF_YES(flags) do {*flags |= __PRINTF_ALTERF; } while (0)
#define __PRINTF_S_ALTERF_NO(flags) do {*flags &= ~__PRINTF_ALTERF; } while (0)

#define __PRINTF_ZEROPAD (1<<5)
#define __PRINTF_ZEROPAD_YES(flags) ((flags & __PRINTF_ZEROPAD) == __PRINTF_ZEROPAD)
#define __PRINTF_ZEROPAD_NO(flags) ((flags & __PRINTF_ZEROPAD) == 0)
#define __PRINTF_S_ZEROPAD_YES(flags) do {*flags |= __PRINTF_ZEROPAD; } while (0)
#define __PRINTF_S_ZEROPAD_NO(flags) do {*flags &= ~__PRINTF_ZEROPAD; } while (0)

int __printf_flags(char** format, uint64_t* flags){
	switch (**format){
	case '-': __PRINTF_S_JUSTIFY_LEFT(flags); break;
	case '+': __PRINTF_S_SIGNCONV_ALLWAYS(flags); break;
	case ' ': __PRINTF_S_SIGNCONV_SPACE_ALLWAYS(flags); break;
	case '#': __PRINTF_S_ALTERF_YES(flags); break;
	case '0': __PRINTF_S_ZEROPAD_YES(flags); break;
	default:
		return 0;
	}
	++*format;
	return 1;
}

#define __PRINTF_CHECKMOD(format, target, spec, type) \
	if (strncmp(format, spec, strlen(spec)) == 0) { target = type; format += strlen(spec); }
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

#define __PRINTF_WRITETO_BODY(type) \
	type* ptr = va_arg(args, type*); \
	*ptr = (type)*writelen; \
	return 0
#define __PRINTF_WRITETO_DEF(tpref, type) \
	if (lenmod == tpref){ \
		__PRINTF_WRITETO_BODY(type); \
	}

#define __PRINTF_NUMCONVS_BODY(type, radix) \
		__DIGIT_SIZE(digit_size, sizeof(type), radix); \
		value = (type)va_arg(args, type)
#define __PRINTF_NUMCONVS_BODYS(type, radix) \
		__DIGIT_SIZE(digit_size, sizeof(type), radix); \
		value = (type)va_arg(args, int)
#define __PRINTF_NUMCONVS_DEF(tpref, type, radix) \
	if (lenmod == tpref) { \
		__PRINTF_NUMCONVS_BODY(type, radix); \
	}
#define __PRINTF_NUMCONVS_DEFS(tpref, type, radix) \
	if (lenmod == tpref) { \
		__PRINTF_NUMCONVS_BODYS(type, radix); \
	}

#define __PRINTF_CONV_INT_TO_SINT 			 	 1
#define __PRINTF_CONV_UINT_TO_UOCT 				10
#define __PRINTF_CONV_UINT_TO_UDEC 				11
#define __PRINTF_CONV_UINT_TO_UHEX_L 			12
#define __PRINTF_CONV_UINT_TO_UHEX_U 			13
#define __PRINTF_CONV_DBL_TO_FLT_DECNOT 		20
#define __PRINTF_CONV_DBL_TO_FLT_DECNOT_U 		21
#define __PRINTF_CONV_DBL_TO_FLT_ENOT 			22
#define __PRINTF_CONV_DBL_TO_FLT_ENOT_U 		23
#define __PRINTF_CONV_DBL_TO_FLT_DECORENOT  	24
#define __PRINTF_CONV_DBL_TO_FLT_DECORENOT_U 	25
#define __PRINTF_CONV_DBL_TO_FLT_HEXNOT 		26
#define __PRINTF_CONV_DBL_TO_FLT_HEXNOT_U 		27
#define __PRINTF_CONV_CHAR 						30
#define __PRINTF_CONV_STRING 					40
#define __PRINTF_CONV_PTR 						50
#define __PRINTF_CONV_WRITETO 					60
#define __PRINTF_CONV_PERC 						70

// predcomputed sizes
static const int const __digits_to_radix[2048] = {
	0, 1, 1, 1, 2, 2, 2, 3, 3, 3, 4, 4, 4, 4, 5, 5, 5, 6, 6, 6, 7, 7, 7, 7, 8, 8, 8, 9, 9, 9, 10, 10, 10, 10, 11, 11, 11, 12, 12, 12, 13, 13, 13, 13, 14, 14, 14, 15, 15, 15, 16, 16, 16, 16, 17, 17, 17, 18, 18, 18, 19, 19, 19, 19, 20, 20, 20, 21, 21, 21, 22, 22, 22, 22, 23, 23, 23, 24, 24, 24, 25, 25, 25, 25, 26, 26, 26, 27, 27, 27, 28, 28, 28, 28, 29, 29, 29, 30, 30, 30, 31, 31, 31, 32, 32, 32, 32, 33, 33, 33, 34, 34, 34, 35, 35, 35, 35, 36, 36, 36, 37, 37, 37, 38, 38, 38, 38, 39, 39, 39, 40, 40, 40, 41, 41, 41, 41, 42, 42, 42, 43, 43, 43, 44, 44, 44, 44, 45, 45, 45, 46, 46, 46, 47, 47, 47, 47, 48, 48, 48, 49, 49, 49, 50, 50, 50, 50, 51, 51, 51, 52, 52, 52, 53, 53, 53, 53, 54, 54, 54, 55, 55, 55, 56, 56, 56, 56, 57, 57, 57, 58, 58, 58, 59, 59, 59, 60, 60, 60, 60, 61, 61, 61, 62, 62, 62, 63, 63, 63, 63, 64, 64, 64, 65, 65, 65, 66, 66, 66, 66, 67, 67, 67, 68, 68, 68, 69, 69, 69, 69, 70, 70, 70, 71, 71, 71, 72, 72, 72, 72, 73, 73, 73, 74, 74, 74, 75, 75, 75, 75, 76, 76, 76, 77, 77, 77, 78, 78, 78, 78, 79, 79, 79, 80, 80, 80, 81, 81, 81, 81, 82, 82, 82, 83, 83, 83, 84, 84, 84, 84, 85, 85, 85, 86, 86, 86, 87, 87, 87, 87, 88, 88, 88, 89, 89, 89, 90, 90, 90, 91, 91, 91, 91, 92, 92, 92, 93, 93, 93, 94, 94, 94, 94, 95, 95, 95, 96, 96, 96, 97, 97, 97, 97, 98, 98, 98, 99, 99, 99, 100, 100, 100, 100, 101, 101, 101, 102, 102, 102, 103, 103, 103, 103, 104, 104, 104, 105, 105, 105, 106, 106, 106, 106, 107, 107, 107, 108, 108, 108, 109, 109, 109, 109, 110, 110, 110, 111, 111, 111, 112, 112, 112, 112, 113, 113, 113, 114, 114, 114, 115, 115, 115, 115, 116, 116, 116, 117, 117, 117, 118, 118, 118, 119, 119, 119, 119, 120, 120, 120, 121, 121, 121, 122, 122, 122, 122, 123, 123, 123, 124, 124, 124, 125, 125, 125, 125, 126, 126, 126, 127, 127, 127, 128, 128, 128, 128, 129, 129, 129, 130, 130, 130, 131, 131, 131, 131, 132, 132, 132, 133, 133, 133, 134, 134, 134, 134, 135, 135, 135, 136, 136, 136, 137, 137, 137, 137, 138, 138, 138, 139, 139, 139, 140, 140, 140, 140, 141, 141, 141, 142, 142, 142, 143, 143, 143, 143, 144, 144, 144, 145, 145, 145, 146, 146, 146, 146, 147, 147, 147, 148, 148, 148, 149, 149, 149, 150, 150, 150, 150, 151, 151, 151, 152, 152, 152, 153, 153, 153, 153, 154, 154, 154, 155, 155, 155, 156, 156, 156, 156, 157, 157, 157, 158, 158, 158, 159, 159, 159, 159, 160, 160, 160, 161, 161, 161, 162, 162, 162, 162, 163, 163, 163, 164, 164, 164, 165, 165, 165, 165, 166, 166, 166, 167, 167, 167, 168, 168, 168, 168, 169, 169, 169, 170, 170, 170, 171, 171, 171, 171, 172, 172, 172, 173, 173, 173, 174, 174, 174, 174, 175, 175, 175, 176, 176, 176, 177, 177, 177, 178, 178, 178, 178, 179, 179, 179, 180, 180, 180, 181, 181, 181, 181, 182, 182, 182, 183, 183, 183, 184, 184, 184, 184, 185, 185, 185, 186, 186, 186, 187, 187, 187, 187, 188, 188, 188, 189, 189, 189, 190, 190, 190, 190, 191, 191, 191, 192, 192, 192, 193, 193, 193, 193, 194, 194, 194, 195, 195, 195, 196, 196, 196, 196, 197, 197, 197, 198, 198, 198, 199, 199, 199, 199, 200, 200, 200, 201, 201, 201, 202, 202, 202, 202, 203, 203, 203, 204, 204, 204, 205, 205, 205, 206, 206, 206, 206, 207, 207, 207, 208, 208, 208, 209, 209, 209, 209, 210, 210, 210, 211, 211, 211, 212, 212, 212, 212, 213, 213, 213, 214, 214, 214, 215, 215, 215, 215, 216, 216, 216, 217, 217, 217, 218, 218, 218, 218, 219, 219, 219, 220, 220, 220, 221, 221, 221, 221, 222, 222, 222, 223, 223, 223, 224, 224, 224, 224, 225, 225, 225, 226, 226, 226, 227, 227, 227, 227, 228, 228, 228, 229, 229, 229, 230, 230, 230, 230, 231, 231, 231, 232, 232, 232, 233, 233, 233, 233, 234, 234, 234, 235, 235, 235, 236, 236, 236, 237, 237, 237, 237, 238, 238, 238, 239, 239, 239, 240, 240, 240, 240, 241, 241, 241, 242, 242, 242, 243, 243, 243, 243, 244, 244, 244, 245, 245, 245, 246, 246, 246, 246, 247, 247, 247, 248, 248, 248, 249, 249, 249, 249, 250, 250, 250, 251, 251, 251, 252, 252, 252, 252, 253, 253, 253, 254, 254, 254, 255, 255, 255, 255, 256, 256, 256, 257, 257, 257, 258, 258, 258, 258, 259, 259, 259, 260, 260, 260, 261, 261, 261, 261, 262, 262, 262, 263, 263, 263, 264, 264, 264, 265, 265, 265, 265, 266, 266, 266, 267, 267, 267, 268, 268, 268, 268, 269, 269, 269, 270, 270, 270, 271, 271, 271, 271, 272, 272, 272, 273, 273, 273, 274, 274, 274, 274, 275, 275, 275, 276, 276, 276, 277, 277, 277, 277, 278, 278, 278, 279, 279, 279, 280, 280, 280, 280, 281, 281, 281, 282, 282, 282, 283, 283, 283, 283, 284, 284, 284, 285, 285, 285, 286, 286, 286, 286, 287, 287, 287, 288, 288, 288, 289, 289, 289, 289, 290, 290, 290, 291, 291, 291, 292, 292, 292, 292, 293, 293, 293, 294, 294, 294, 295, 295, 295, 296, 296, 296, 296, 297, 297, 297, 298, 298, 298, 299, 299, 299, 299, 300, 300, 300, 301, 301, 301, 302, 302, 302, 302, 303, 303, 303, 304, 304, 304, 305, 305, 305, 305, 306, 306, 306, 307, 307, 307, 308, 308, 308, 308, 309, 309, 309, 310, 310, 310, 311, 311, 311, 311, 312, 312, 312, 313, 313, 313, 314, 314, 314, 314, 315, 315, 315, 316, 316, 316, 317, 317, 317, 317, 318, 318, 318, 319, 319, 319, 320, 320, 320, 320, 321, 321, 321, 322, 322, 322, 323, 323, 323, 324, 324, 324, 324, 325, 325, 325, 326, 326, 326, 327, 327, 327, 327, 328, 328, 328, 329, 329, 329, 330, 330, 330, 330, 331, 331, 331, 332, 332, 332, 333, 333, 333, 333, 334, 334, 334, 335, 335, 335, 336, 336, 336, 336, 337, 337, 337, 338, 338, 338, 339, 339, 339, 339, 340, 340, 340, 341, 341, 341, 342, 342, 342, 342, 343, 343, 343, 344, 344, 344, 345, 345, 345, 345, 346, 346, 346, 347, 347, 347, 348, 348, 348, 348, 349, 349, 349, 350, 350, 350, 351, 351, 351, 352, 352, 352, 352, 353, 353, 353, 354, 354, 354, 355, 355, 355, 355, 356, 356, 356, 357, 357, 357, 358, 358, 358, 358, 359, 359, 359, 360, 360, 360, 361, 361, 361, 361, 362, 362, 362, 363, 363, 363, 364, 364, 364, 364, 365, 365, 365, 366, 366, 366, 367, 367, 367, 367, 368, 368, 368, 369, 369, 369, 370, 370, 370, 370, 371, 371, 371, 372, 372, 372, 373, 373, 373, 373, 374, 374, 374, 375, 375, 375, 376, 376, 376, 376, 377, 377, 377, 378, 378, 378, 379, 379, 379, 379, 380, 380, 380, 381, 381, 381, 382, 382, 382, 383, 383, 383, 383, 384, 384, 384, 385, 385, 385, 386, 386, 386, 386, 387, 387, 387, 388, 388, 388, 389, 389, 389, 389, 390, 390, 390, 391, 391, 391, 392, 392, 392, 392, 393, 393, 393, 394, 394, 394, 395, 395, 395, 395, 396, 396, 396, 397, 397, 397, 398, 398, 398, 398, 399, 399, 399, 400, 400, 400, 401, 401, 401, 401, 402, 402, 402, 403, 403, 403, 404, 404, 404, 404, 405, 405, 405, 406, 406, 406, 407, 407, 407, 407, 408, 408, 408, 409, 409, 409, 410, 410, 410, 411, 411, 411, 411, 412, 412, 412, 413, 413, 413, 414, 414, 414, 414, 415, 415, 415, 416, 416, 416, 417, 417, 417, 417, 418, 418, 418, 419, 419, 419, 420, 420, 420, 420, 421, 421, 421, 422, 422, 422, 423, 423, 423, 423, 424, 424, 424, 425, 425, 425, 426, 426, 426, 426, 427, 427, 427, 428, 428, 428, 429, 429, 429, 429, 430, 430, 430, 431, 431, 431, 432, 432, 432, 432, 433, 433, 433, 434, 434, 434, 435, 435, 435, 435, 436, 436, 436, 437, 437, 437, 438, 438, 438, 438, 439, 439, 439, 440, 440, 440, 441, 441, 441, 442, 442, 442, 442, 443, 443, 443, 444, 444, 444, 445, 445, 445, 445, 446, 446, 446, 447, 447, 447, 448, 448, 448, 448, 449, 449, 449, 450, 450, 450, 451, 451, 451, 451, 452, 452, 452, 453, 453, 453, 454, 454, 454, 454, 455, 455, 455, 456, 456, 456, 457, 457, 457, 457, 458, 458, 458, 459, 459, 459, 460, 460, 460, 460, 461, 461, 461, 462, 462, 462, 463, 463, 463, 463, 464, 464, 464, 465, 465, 465, 466, 466, 466, 466, 467, 467, 467, 468, 468, 468, 469, 469, 469, 470, 470, 470, 470, 471, 471, 471, 472, 472, 472, 473, 473, 473, 473, 474, 474, 474, 475, 475, 475, 476, 476, 476, 476, 477, 477, 477, 478, 478, 478, 479, 479, 479, 479, 480, 480, 480, 481, 481, 481, 482, 482, 482, 482, 483, 483, 483, 484, 484, 484, 485, 485, 485, 485, 486, 486, 486, 487, 487, 487, 488, 488, 488, 488, 489, 489, 489, 490, 490, 490, 491, 491, 491, 491, 492, 492, 492, 493, 493, 493, 494, 494, 494, 494, 495, 495, 495, 496, 496, 496, 497, 497, 497, 498, 498, 498, 498, 499, 499, 499, 500, 500, 500, 501, 501, 501, 501, 502, 502, 502, 503, 503, 503, 504, 504, 504, 504, 505, 505, 505, 506, 506, 506, 507, 507, 507, 507, 508, 508, 508, 509, 509, 509, 510, 510, 510, 510, 511, 511, 511, 512, 512, 512, 513, 513, 513, 513, 514, 514, 514, 515, 515, 515, 516, 516, 516, 516, 517, 517, 517, 518, 518, 518, 519, 519, 519, 519, 520, 520, 520, 521, 521, 521, 522, 522, 522, 522, 523, 523, 523, 524, 524, 524, 525, 525, 525, 525, 526, 526, 526, 527, 527, 527, 528, 528, 528, 529, 529, 529, 529, 530, 530, 530, 531, 531, 531, 532, 532, 532, 532, 533, 533, 533, 534, 534, 534, 535, 535, 535, 535, 536, 536, 536, 537, 537, 537, 538, 538, 538, 538, 539, 539, 539, 540, 540, 540, 541, 541, 541, 541, 542, 542, 542, 543, 543, 543, 544, 544, 544, 544, 545, 545, 545, 546, 546, 546, 547, 547, 547, 547, 548, 548, 548, 549, 549, 549, 550, 550, 550, 550, 551, 551, 551, 552, 552, 552, 553, 553, 553, 553, 554, 554, 554, 555, 555, 555, 556, 556, 556, 557, 557, 557, 557, 558, 558, 558, 559, 559, 559, 560, 560, 560, 560, 561, 561, 561, 562, 562, 562, 563, 563, 563, 563, 564, 564, 564, 565, 565, 565, 566, 566, 566, 566, 567, 567, 567, 568, 568, 568, 569, 569, 569, 569, 570, 570, 570, 571, 571, 571, 572, 572, 572, 572, 573, 573, 573, 574, 574, 574, 575, 575, 575, 575, 576, 576, 576, 577, 577, 577, 578, 578, 578, 578, 579, 579, 579, 580, 580, 580, 581, 581, 581, 581, 582, 582, 582, 583, 583, 583, 584, 584, 584, 584, 585, 585, 585, 586, 586, 586, 587, 587, 587, 588, 588, 588, 588, 589, 589, 589, 590, 590, 590, 591, 591, 591, 591, 592, 592, 592, 593, 593, 593, 594, 594, 594, 594, 595, 595, 595, 596, 596, 596, 597, 597, 597, 597, 598, 598, 598, 599, 599, 599, 600, 600, 600, 600, 601, 601, 601, 602, 602, 602, 603, 603, 603, 603, 604, 604, 604, 605, 605, 605, 606, 606, 606, 606, 607, 607, 607, 608, 608, 608, 609, 609, 609, 609, 610, 610, 610, 611, 611, 611, 612, 612, 612, 612, 613, 613, 613, 614, 614, 614, 615, 615, 615, 616, 616, 616, 616, 617
};

#define __DIGIT_SIZE(result, bytesize, radix) \
	do { \
		if (radix == 10) result = __digits_to_radix[bytesize*8]; \
		else if (radix == 8) result = (bytesize*8 / 4) * 3; \
		else if (radix == 16) result = (bytesize*8 / 4); \
	} while (0)

#define __PRINTF_CONV_TO_RADIX(conv) \
	((conv == __PRINTF_CONV_INT_TO_SINT || conv == __PRINTF_CONV_UINT_TO_UDEC) ? 10 : \
	((conv == __PRINTF_CONV_UINT_TO_UOCT) ? 8 : \
	   16))

char __valradix(uint8_t v, bool uppercase){
	static char val_radix_upper[] = {
					'0', '1', '2', '3', '4', '5', '6', '7', '8', '9',
					'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H', 'I', 'J', 'K', 'L', 'M', 'N', 'O', 'P', 'Q', 'R', 'S', 'T', 'U', 'V', 'W', 'X', 'Y', 'Z'
			};
	static char val_radix_lower[] = {
			'0', '1', '2', '3', '4', '5', '6', '7', '8', '9',
			'a', 'b', 'c', 'd', 'e', 'f', 'g', 'h', 'i', 'j', 'k', 'l', 'm', 'n', 'o', 'p', 'q', 'r', 's', 't', 'u', 'v', 'w', 'x', 'y', 'z'
	};
	if (uppercase)
		return val_radix_upper[v];
	else
		return val_radix_lower[v];
}

int __vprintf_arg(FILE* stream, char** fplace, size_t* writelen,
					uint64_t flags, int widthspec, int precision, uint64_t lenmod, uint64_t type, va_list args){
	if (type == __PRINTF_CONV_PERC){
		__PRINTF_WRITE("%", stream, 1);
		++*writelen;
		return 0;
	}

	if (type == __PRINTF_CONV_WRITETO){
		__PRINTF_WRITETO_DEF(__PRINTF_LENMOD_HH, signed char);
		__PRINTF_WRITETO_DEF(__PRINTF_LENMOD_H, short int);
		__PRINTF_WRITETO_DEF(__PRINTF_LENMOD_L, long int);
		__PRINTF_WRITETO_DEF(__PRINTF_LENMOD_LL, long long int);
		__PRINTF_WRITETO_DEF(__PRINTF_LENMOD_J, intmax_t);
		__PRINTF_WRITETO_DEF(__PRINTF_LENMOD_Z, size_t);
		__PRINTF_WRITETO_DEF(__PRINTF_LENMOD_T, ptrdiff_t);
		__PRINTF_WRITETO_BODY(int);
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

	if (type == __PRINTF_CONV_INT_TO_SINT){
		size_t digit_size = 0;
		intmax_t value;

		__PRINTF_NUMCONVS_DEFS(__PRINTF_LENMOD_HH, signed char, __PRINTF_CONV_TO_RADIX(type))
		else __PRINTF_NUMCONVS_DEFS(__PRINTF_LENMOD_H, short int, __PRINTF_CONV_TO_RADIX(type))
		else __PRINTF_NUMCONVS_DEF(__PRINTF_LENMOD_L, long int, __PRINTF_CONV_TO_RADIX(type))
		else __PRINTF_NUMCONVS_DEF(__PRINTF_LENMOD_LL, long long int, __PRINTF_CONV_TO_RADIX(type))
		else __PRINTF_NUMCONVS_DEF(__PRINTF_LENMOD_J, intmax_t, __PRINTF_CONV_TO_RADIX(type))
		else __PRINTF_NUMCONVS_DEF(__PRINTF_LENMOD_Z, size_t, __PRINTF_CONV_TO_RADIX(type))
		else __PRINTF_NUMCONVS_DEF(__PRINTF_LENMOD_T, ptrdiff_t, __PRINTF_CONV_TO_RADIX(type))
		else { __PRINTF_NUMCONVS_BODY(int, __PRINTF_CONV_TO_RADIX(type)); }

		digit_size += 1; // for sign
		char buffer[digit_size], rbuffer[digit_size];
		memset(buffer, 0, digit_size);

		if (value == 0 && precision == 0){
			__PRINTF_WRITE("0", stream, 1);
			*writelen += 1;
			return 0;
		}

		bool sign = false;
		if (value < 0){
			value *= -1;
			sign = true;
		}

		size_t used_bytes = 0;
		do {
			char digit = value % 10;
			value /= 10;
			buffer[used_bytes++] = __valradix(digit, 0);
		} while (value != 0);

		bool printed_sign = false;
		if (precision != 0){
			if (__PRINTF_SIGNCONV_ALLWAYS(flags)){
				buffer[used_bytes] = sign ? '-' : '+';
				printed_sign = true;
			} else if (__PRINTF_SIGNCONV_SPACE_ALLWAYS(flags)){
				buffer[used_bytes] = sign ? '-' : ' ';
				printed_sign = true;
			} else if (sign){
				buffer[used_bytes] = '-';
				printed_sign = true;
			}
		} else if (__PRINTF_SIGNCONV_SPACE_ALLWAYS(flags) && !__PRINTF_SIGNCONV_NEGATIVE(flags)){
			buffer[used_bytes] = ' ';
			printed_sign = true;
		}

		if (__PRINTF_ZEROPAD_YES(flags) && !__PRINTF_JUSTIFY_RIGHT(flags)){
			precision = widthspec - (printed_sign ? 1 : 0);
			if (precision < 0)
				precision = 0;
		}

		// reverse the buffer
		for (size_t i=0; i<=used_bytes; i++){
			rbuffer[i] = buffer[used_bytes-i];
		}

		size_t writesize = (used_bytes < (size_t)precision ? (size_t)precision : used_bytes) + (printed_sign ? 1 : 0);
		if (widthspec >= 0){
			writesize = widthspec;
		}

		if (writesize <= used_bytes+1){
			if (printed_sign && writesize <= used_bytes)
				__PRINTF_WRITE(rbuffer+1, stream, writesize);
			else if (printed_sign)
				__PRINTF_WRITE(rbuffer, stream, writesize);
			else
				__PRINTF_WRITE(rbuffer+1, stream, writesize);
		} else {
			size_t diff = (size_t)precision - used_bytes;
			if ((size_t)precision < used_bytes)
				diff = 0;
			size_t difference = writesize - (used_bytes+(printed_sign ? 1 : 0));
			if (difference < diff) {
				diff = difference;
				difference = 0;
			} else
				difference -= diff;
			if (__PRINTF_JUSTIFY_LEFT(flags)){
				// print left justify
				for (unsigned int i=0; i<difference; i++)
					__PRINTF_WRITE(" ", stream, 1);
			}
			if (printed_sign)
				__PRINTF_WRITE(rbuffer, stream, 1); // print sign
			if (used_bytes < (size_t)precision){
				// print filling
				for (unsigned int i=0; i<diff; i++)
					__PRINTF_WRITE("0", stream, 1);
			}
			__PRINTF_WRITE(rbuffer+1, stream, used_bytes); // print data
			if (__PRINTF_JUSTIFY_RIGHT(flags)){
				// print right justify
				for (unsigned int i=0; i<difference; i++)
					__PRINTF_WRITE(" ", stream, 1);
			}
		}

		*writelen += writesize;
		return 0;
	}

	if (type >= __PRINTF_CONV_UINT_TO_UOCT && type <= __PRINTF_CONV_UINT_TO_UHEX_U){
		size_t digit_size = 0;
		uintmax_t value;

		__PRINTF_NUMCONVS_DEFS(__PRINTF_LENMOD_HH, unsigned char, __PRINTF_CONV_TO_RADIX(type))
		else __PRINTF_NUMCONVS_DEFS(__PRINTF_LENMOD_H, unsigned short int, __PRINTF_CONV_TO_RADIX(type))
		else __PRINTF_NUMCONVS_DEF(__PRINTF_LENMOD_L, unsigned long int, __PRINTF_CONV_TO_RADIX(type))
		else __PRINTF_NUMCONVS_DEF(__PRINTF_LENMOD_LL, unsigned long long int, __PRINTF_CONV_TO_RADIX(type))
		else __PRINTF_NUMCONVS_DEF(__PRINTF_LENMOD_J, uintmax_t, __PRINTF_CONV_TO_RADIX(type))
		else __PRINTF_NUMCONVS_DEF(__PRINTF_LENMOD_Z, size_t, __PRINTF_CONV_TO_RADIX(type))
		else __PRINTF_NUMCONVS_DEF(__PRINTF_LENMOD_T, ptrdiff_t, __PRINTF_CONV_TO_RADIX(type))
		else { __PRINTF_NUMCONVS_BODY(unsigned int, __PRINTF_CONV_TO_RADIX(type)); }

		digit_size += 2; // for 0x
		char buffer[digit_size], rbuffer[digit_size];
		memset(buffer, 0, digit_size);

		size_t used_bytes = 0;
		uint8_t radix = __PRINTF_CONV_TO_RADIX(type);
		uintmax_t vvalue = value;
		do {
			char digit = vvalue % radix;
			vvalue /= radix;
			buffer[used_bytes++] = __valradix(digit, type == __PRINTF_CONV_UINT_TO_UHEX_U);
		} while (vvalue != 0);

		if (__PRINTF_ZEROPAD_YES(flags) && !__PRINTF_JUSTIFY_RIGHT(flags)){
			precision = widthspec;
			if (precision < 0)
				precision = 0;
		}

		if (__PRINTF_ALTERF_YES(flags)){
			if (type == __PRINTF_CONV_UINT_TO_UOCT){
				if (precision == 0 && value == 0){
					__PRINTF_WRITE("0", stream, 1);
					return 0;
				}

				if ((size_t)precision <= used_bytes && buffer[used_bytes-1] != '0'){
					precision = used_bytes+1;
				}
			} else if ((type == __PRINTF_CONV_UINT_TO_UHEX_L || type == __PRINTF_CONV_UINT_TO_UHEX_U) && value != 0){
				buffer[used_bytes++] = (type == __PRINTF_CONV_UINT_TO_UHEX_U ? 'X' : 'x');
				buffer[used_bytes++] = '0';
			}
		}

		if (value == 0 && precision == 0){
			return 0;
		}

		// reverse the buffer
		for (size_t i=0; i<used_bytes; i++){
			rbuffer[i] = buffer[used_bytes-i-1];
		}

		size_t writesize = (used_bytes < (size_t)precision ? (size_t)precision : used_bytes);
		if (widthspec >= 0){
			writesize = widthspec;
		}

		if (writesize <= used_bytes){
			__PRINTF_WRITE(rbuffer, stream, writesize);
		} else {
			size_t diff = (size_t)precision - used_bytes;
			if ((size_t)precision < used_bytes)
				diff = 0;
			size_t difference = writesize - used_bytes;
			if (difference < diff) {
				diff = difference;
				difference = 0;
			} else
				difference -= diff;
			if (__PRINTF_JUSTIFY_LEFT(flags)){
				// print left justify
				for (unsigned int i=0; i<difference; i++)
					__PRINTF_WRITE(" ", stream, 1);
			}
			if (used_bytes < (size_t)precision){
				// print filling
				for (unsigned int i=0; i<diff; i++)
					__PRINTF_WRITE("0", stream, 1);
			}
			__PRINTF_WRITE(rbuffer, stream, used_bytes); // print data
			if (__PRINTF_JUSTIFY_RIGHT(flags)){
				// print right justify
				for (unsigned int i=0; i<difference; i++)
					__PRINTF_WRITE(" ", stream, 1);
			}
		}

		*writelen += writesize;
		return 0;
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
				if (widthspec < 0)
					widthspec = -widthspec;
				++fmt;
			} else {
				if (widthspec < 0)
					widthspec = -widthspec;
				widthspec = (int)strtol((const char*)fmt, &fmt, 10);
				if (errno == ERANGE){
					errno = 0;
					return __FORMAT_ERROR_LENSPEC_INCORRECT;
				}
			}

			__CHECK_INVALID(fmt);

			int precision = 1;
			if (*fmt == '.'){
				++fmt;
				if (*fmt == '*') {
					precision = va_arg(arg, int);
					if (precision < 0)
						precision = -precision;
					++fmt;
				} else {
					if (precision < 0)
						precision = -precision;
					precision = (int)strtol((const char*)fmt, &fmt, 10);
					if (errno == ERANGE){
						errno = 0;
						return __FORMAT_ERROR_PERCISION_INCORRECT;
					}
				}
				__PRINTF_S_ZEROPAD_NO(&flags);
			}

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

			int retval = __vprintf_arg(stream, &fmt, &written_amount, flags, widthspec, precision, lenmod, type, arg);
			if (retval)
				return retval;
		}
	}

	return 0;
}
