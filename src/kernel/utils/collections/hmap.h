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
 * hmap.h
 *  Created on: Jan 3, 2016
 *      Author: Peter Vanusanik
 *  Contents: 
 */

#pragma once

#include "../../commons.h"
#include "funcops.h"

#include <stdlib.h>
#include <string.h>

#define HASH_STARTING_SIZE 97

#define FREE_CELL               0
#define DELETED_CELL    ((void*)1)

typedef struct {
    void* key;
    void* data;
} hash_pair_t;

typedef struct {
    hash_pair_t** hash_table;
    hash_function_t hash_fn;
    eq_function_t cmpr_fn;
    uint32_t len;
    uint32_t max_size;
} hash_table_t;

typedef uint32_t hash_it_t;

hash_table_t* create_table(hash_function_t hash_fn, eq_function_t cmpr_fn);

void destroy_table(hash_table_t* table);

bool table_contains(hash_table_t* table, void* key);

void* table_get(hash_table_t* table, void* key);

void table_set(hash_table_t* table, void* key, void* data);

bool table_remove(hash_table_t* table, void* key);

uint32_t table_size(hash_table_t* table);

hash_table_t* copy_table(hash_table_t* table);

void* hash_it_next(hash_table_t* table, hash_it_t* iterator);

/* helper constructor functions */

hash_table_t* create_uint32_table();
hash_table_t* create_uint64_table();
hash_table_t* create_string_table();
