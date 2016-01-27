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
 * drivers.h
 *  Created on: Jan 27, 2016
 *      Author: Peter Vanusanik
 *  Contents: 
 */

#pragma once

#include <stdbool.h>

#include <ny/nyarlathotep.h>
#include <datastruct/array.h>

#define SATA "sata"

typedef struct dlist_entry {
	char* dtype;
	char* path;
} dlist_entry_t;

typedef dlist_entry_t* dlist_entry_ptr_t;
ARRAY_HEADER(dlist_entry_ptr_t)
typedef ARRAY_TYPE(dlist_entry_ptr_t) dlist_entry_array;

void load_from_initramfs(const char* path);
void load_from_disk(const char* path);

extern dlist_entry_array* drivers;
