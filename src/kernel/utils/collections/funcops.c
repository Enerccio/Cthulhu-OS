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
 * funcops.c
 *  Created on: Jan 3, 2016
 *      Author: Peter Vanusanik
 *  Contents: 
 */

#include "funcops.h"

/**
 * Converts uint32_t in void* into hash.
 */
uint32_t uint32_hash_function(void* integer) {
    uint32_t a = (uint32_t) ((uintptr_t)integer);
    a = (a + 0x7ed55d16) + (a << 12);
    a = (a ^ 0xc761c23c) ^ (a >> 19);
    a = (a + 0x165667b1) + (a << 5);
    a = (a + 0xd3a2646c) ^ (a << 9);
    a = (a + 0xfd7046c5) + (a << 3);
    a = (a ^ 0xb55a4f09) ^ (a >> 16);
    return a;
}

/**
 * Integer comparisons, of void* arguments representing integer.
 */
bool uint32_eq_function(void* a, void* b) {
    return a == b ? true : false;
}

uint32_t uint64_hash_function(void* integer) {
    uint64_t a = (uint64_t)integer;
    return uint32_hash_function((void*)(uintptr_t)(a >> 32 & 0xFFFFFFFF)) ^
            uint32_hash_function((void*)(uintptr_t)(a & 0xFFFFFFFF));
}

bool uint64_eq_function(void* a, void* b) {
    return a == b ? true : false;
}

uint32_t string_hash_function(void* string) {
    char* str = string;
    uint32_t hash = 5381;
    char c;

    while ((c = *str++))
        hash = ((hash << 5) + hash) + c;

    return hash;
}

bool string_eq_function(void* a, void* b) {
    return strcmp((char*)a, (char*)b) == 0;
}
