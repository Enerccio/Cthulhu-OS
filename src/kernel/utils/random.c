/*
 * The MIT License (MIT)
 * Copyright (c) 2015 Peter Vanusanik <admin@en-circle.com>
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy 
 * of this software and associated documentation files (the "Software"), to deal in 
 * the Software without restriction, including without limitation the rights to use, copy, 
 * modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, 
 * and to permit persons to whom the Software is furnished to do so, subject to the 
 * following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all copies 
 * or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, 
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, 
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS 
 * OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, 
 * WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN 
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 *
 * random.c
 *  Created on: Dec 23, 2015
 *      Author: Peter Vanusanik
 *  Contents: random generator based on xorshift
 */
#include "random.h"

rg_t rg_create_random_generator(uint64_t seed){
	rg_t rg;
	rg.state[0] = (seed & 0xFFFFFFFF00000000) + ((seed << 32) ^ seed);
	rg.state[1] = (seed & 0x00000000FFFFFFFF) + ((seed >> 32) ^ seed);
}

uint64_t rg_next_int(rg_t rg){
	uint64_t x = rg.state[0];
	uint64_t const y = rg.state[1];
	rg.state[0] = y;
	x ^= x << 23; // a
	rg.state[1] = x ^ y ^ (x >> 17) ^ (y >> 26); // b, c
	return rg.state[1] + y;
}

uint64_t rg_next_int_l(rg_t rg, uint64_t limit){
	return rg_next_int(rg) % limit;
}
