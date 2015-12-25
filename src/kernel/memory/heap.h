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
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED,
 * INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR
 * PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE
 * FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR
 * OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 *
 * heap.h
 *  Created on: Dec 22, 2015
 *      Author: Peter Vanusanik
 *  Contents: kernel heap organization
 */
#pragma once

#include "../commons.h"
#include "../utils/rsod.h"
#include "paging.h"

#include <stdlib.h>

/**
 * Initializes temporary heap with provided address
 */
void initialize_temporary_heap(uint64_t temp_heap_start);
/**
 * Initializes standard heap
 */
void initialize_standard_heap();
/**
 * Returns allocated address which is aligned to align parameter
 */
void* malign(size_t amount, uint16_t align);

/**
 * Start of the heap address
 */
extern uint64_t heap_start_address;
