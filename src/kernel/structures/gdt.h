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
 * gdt.h
 *  Created on: Dec 28, 2015
 *      Author: Peter Vanusanik
 *  Contents: 
 */

#pragma once

#include "../commons.h"
#include "tss.h"

#include <stdlib.h>

typedef struct __attribute__((packed)) descriptor {
    uint64_t limit0015 : 16;
    uint64_t base0015  : 16;
    uint64_t base2316  : 8;
    uint64_t type      : 4;
    uint64_t s         : 1;
    uint64_t dpl       : 2;
    uint64_t p         : 1;
    uint64_t limit1916 : 4;
    uint64_t avl       : 1;
    uint64_t l         : 1;
    uint64_t db        : 1;
    uint64_t g         : 1;
    uint64_t base3124  : 8;
} descriptor_t;

typedef struct __attribute__((packed)) tss_descriptor  {
    descriptor_t descriptor;
    uint32_t base6332;
    uint32_t reserved;
} tss_descriptor_t;

typedef struct __attribute__((packed)) gdt_ptr {
    uint16_t limit;
    descriptor_t* descriptors;
} gdt_ptr_t;

extern gdt_ptr_t gdt;

void reinitialize_gdt();
