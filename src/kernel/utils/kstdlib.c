/*
 * The MIT License (MIT)
 * Copyright (c) 2015 Peter Vanusanik <admin@en-circle.com>
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the Software
 * is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED,
 * INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR
 * PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE
 * FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR
 * OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 *
 * kstdlib.c
 *  Created on: Dec 22, 2015
 *      Author: Peter Vanusanik
 *  Contents: kernel only helper functions
 */
#include "kstdlib.h"

inline uint32_t rand_number(int32_t limit) {
	static uint32_t a = 3;
	a = (((a * 214013L + 2531011L) >> 16) & 32767);
	return ((a % limit));
}

static char bytetohex(uint8_t byte) {
	switch (byte) {
	case 10:
		return 'A';
	case 11:
		return 'B';
	case 12:
		return 'C';
	case 13:
		return 'D';
	case 14:
		return 'E';
	case 15:
		return 'F';
	default:
		return '0' + byte;
	}
}

char conv[11];

const char* hextochar(uint32_t num) {

	conv[0] = '0';
	conv[1] = 'x';
	conv[2] = bytetohex((num / (1 << 24)) / 16);
	conv[3] = bytetohex((num / (1 << 24)) % 16);
	conv[4] = bytetohex((num / (1 << 16)) / 16);
	conv[5] = bytetohex((num / (1 << 16)) % 16);
	conv[6] = bytetohex((num / (1 << 8)) / 16);
	conv[7] = bytetohex((num / (1 << 8)) % 16);
	conv[8] = bytetohex((num % 256) / 16);
	conv[9] = bytetohex((num % 256) % 16);
	conv[10] = 0;

	return conv;
}

uint32_t int_hash_function(void* integer) {
	uint32_t a = (uint32_t) ((uint64_t)integer);
	a = (a + 0x7ed55d16) + (a << 12);
	a = (a ^ 0xc761c23c) ^ (a >> 19);
	a = (a + 0x165667b1) + (a << 5);
	a = (a + 0xd3a2646c) ^ (a << 9);
	a = (a + 0xfd7046c5) + (a << 3);
	a = (a ^ 0xb55a4f09) ^ (a >> 16);
	return a;
}

bool int_cmpr_function(void* a, void* b) {
	return a == b ? true : false;
}

