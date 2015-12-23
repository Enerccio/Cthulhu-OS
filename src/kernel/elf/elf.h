/*
 * The MIT License (MIT)
 * Copyright (c) 2015 Peter Vanusanik <admin@en-circle.com>
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 *  of this software and associated documentation files (the "Software"), to deal
 *  in the Software without restriction, including without limitation the rights
 *  to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 *  copies of the Software, and to permit persons to whom the Software is furnished
 *  to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all copies
 *  or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED,
 * INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR
 * PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE
 * FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,
 * ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 *
 * elf.h
 *  Created on: Dec 22, 2015
 *      Author: Peter Vanusanik
 *  Contents: elf loader
 */
#pragma once

#include "../commons.h"
#include "../utils/kstdlib.h"
#include "elfdef.h"
#include "../utils/rsod.h"

typedef void  (*elf_memory_requirer_t)(void* start, size_t len);
typedef void* (*elf_memory_allocator_t)(size_t len, bool align);

typedef struct ealloc {
    elf_memory_requirer_t  requirer;
    elf_memory_allocator_t allocator;
} ealloc_t;

#define ELF_HEADER_INCORRECT        0
#define ELF_HEADER_NO_ENTRY         1
#define ELF_KP_EMPTY                2
#define ELF_NOT_64                  3
#define ELF_ENCODING_FAILURE        4
#define ELF_VERSION_FAILURE         5
#define ELF_NO_EXEC_EXEC_REQUIRED   6
#define ELF_RELOC_ERROR             7
#define ELF_NO_SECTIONS             8
#define ELF_ERROR_FAILED_RELOC      9
#define ELF_RELOC_UNSUP_TYPE        10


bool elf_load_kernel(Elf64_Ehdr* header, uint64_t* entry, ealloc_t alloc);
