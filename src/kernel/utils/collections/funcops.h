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
 * funcops.h
 *  Created on: Jan 3, 2016
 *      Author: Peter Vanusanik
 *  Contents: 
 */

#pragma once

#include "../../commons.h"

typedef bool (*search_predicate_t)(void* element, void* passed_data);
typedef int  (*cmp_func_t)(const void* a, const void* b);
typedef void (*key_deallocator_func_t)(void* a);
typedef void (*value_deallocator_func_t)(void* a);

typedef uint32_t (* hash_function_t) (void*);
typedef bool (* eq_function_t) (void*, void*);

/* Concrete func definitions */
uint32_t uint32_hash_function(void* integer);
bool uint32_eq_function(void* a, void* b);
uint32_t uint64_hash_function(void* integer);
bool uint64_eq_function(void* a, void* b);
uint32_t string_hash_function(void* string);
bool string_eq_function(void* a, void* b);
