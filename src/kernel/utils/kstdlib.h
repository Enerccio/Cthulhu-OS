/*
 * The MIT License (MIT)
 * Copyright (c) 2015 Peter Vanusanik <admin@en-circle.com>
 *
 * Permission is hereby granted, free of charge, to any person obtaining
 * a copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation the
 * rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is furnished
 * to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all copies
 * or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED,
 * INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR
 * PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE
 * FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,
 * ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 *
 * kstdlib.h
 *  Created on: Dec 22, 2015
 *      Author: Peter Vanusanik
 *  Contents: kernel only helper functions
 */
#pragma once

#include "../commons.h"

/**
 * Returns simple random number
 */
uint32_t rand_number(int32_t limit);

/**
 * Converts hex number to string, using static buffers.
 * Is not thread safe.
 */
const char* hextochar(uint32_t num) UNSAFE_THREAD_CALL SINGLETON_RETURN;

/**
 * Returns module from multiboot.
 *
 * Address would either be physical_to_virtual, or
 * if reallocate is true, new location in kernel heap will be allocated
 * and module will be copied into it. Then module will be turned into pool
 * if delete is true. reallocate is false and delete is true returns undefined
 * value.
 *
 */
void* get_module(struct multiboot_info* mbheader, const char* name,
        size_t* size, bool reallocate, bool delete);

/**
 * Returns extension of the filename (last part after .) if any, or whole string
 */
char* get_extension(char* fname);
