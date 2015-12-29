/*
 * The MIT License (MIT)
 * Copyright (c) 2015 Peter Vanusanik <admin@en-circle.com>
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation the
 * rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is furnished
 * to do so, subject to the following conditions:
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
 * commons.h
 *  Created on: Dec 22, 2015
 *      Author: Peter Vanusanik
 *  Contents: base kernel information
 */
#pragma once

#define KERNEL_MAIN_VERSION  "0"
#define KERNEL_MINOR_VESION  "0"
#define KERNEL_BUILD_VERSION "1"
#define KERNEL_CODENAME      "Restless Dreams"

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include "utils/textinput.h"
#include "utils/logger.h"

#include "multiboot.h"

#define ENABLE_INTERRUPTS() do { __asm __volatile__ ("sti"); } while (0)
#define DISABLE_INTERRUPTS() do { __asm __volatile__ ("cli"); } while (0)

typedef int8_t cmpresult_t;

#define UNSAFE_THREAD_CALL
#define SINGLETON_RETURN

#ifndef ALIGN_AMOUNT
#define ALIGN_AMOUNT 4096
#endif

typedef void* aligned_ptr_t;

#define RESERVED_KBLOCK_REVERSE_MAPPINGS (510UL)
#define RESERVED_KBLOCK_RAM_MAPPINGS     (509UL)
#define RESERVED_KBLOCK_KHEAP_MAPPINGS   (508UL)

#define ADDRESS_OFFSET(offset) (0xFFFF000000000000UL + (offset<<39))

#define debug_break do { \
		volatile bool _debug_var = true; \
		while (_debug_var) \
			; \
	} while (0)

#define debug_cond(cond) do { \
	if (cond) { \
		debug_break; \
	} while (0)

