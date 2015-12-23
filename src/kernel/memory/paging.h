/*
 * The MIT License (MIT)
 * Copyright (c) 2015 Peter Vanusanik <admin@en-circle.com>
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies
 * of the Software, and to permit persons to whom the Software is furnished to do so,
 * subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all copies
 * or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED,
 * INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR
 * PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE
 * FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR
 * OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 *
 * paging.h
 *  Created on: Dec 22, 2015
 *      Author: Peter Vanusanik
 *  Contents: paging memory for x86-64
 */
#pragma once

#include "../commons.h"
#include "../utils/rsod.h"

#include <limits.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#define BITMASK(b) (1 << ((b) % CHAR_BIT))
#define BITSLOT(b) ((b) / CHAR_BIT)
#define BITSET(a, b) ((a)[BITSLOT(b)] |= BITMASK(b))
#define BITCLEAR(a, b) ((a)[BITSLOT(b)] &= ~BITMASK(b))
#define BITTEST(a, b) ((a)[BITSLOT(b)] & BITMASK(b))
#define BITNSLOTS(nb) ((nb + CHAR_BIT - 1) / CHAR_BIT)

typedef union page {
	struct {
		uint64_t present	: 1;  // whether the page is present
		uint64_t rw			: 1;  // 0 = writes are not allowed, 1 = writers are allowed
		uint64_t us			: 1;  // 0 = CPL3 access is not allowed
		uint64_t pwt		: 1;  // page-level write-through
		uint64_t pcd		: 1;  // page-level cache disable
		uint64_t accessed	: 1;  // whether it was accessed
		uint64_t dirty		: 1;  // whether it was written to
		uint64_t pat		: 1;  // must be 0 unless PAT is supported
		uint64_t global		: 1;  // global if PGE=1, ignored otherwise
		uint64_t ignored    : 3;
		uint64_t address	: 50; // must be 0x1000 aligned
		uint64_t reserved	: 1;
		uint64_t xd			: 1;  // if A32_EFER.NXE = 1, execute-disable -> 1-can't be executed, must be 0 if not enabled
	} 						flaggable;
	uint64_t 				address;
} page_t;

typedef union page_table {
	struct {
		uint64_t present	: 1;  // whether the page is present
		uint64_t rw			: 1;  // 0 = writes are not allowed, 1 = writers are allowed
		uint64_t us			: 1;  // 0 = CPL3 access is not allowed
		uint64_t pwt		: 1;  // page-level write-through
		uint64_t pcd		: 1;  // page-level cache disable
		uint64_t accessed	: 1;  // whether it was accessed
		uint64_t ignored	: 1;
		uint64_t ps			: 1;  // must be 0 since this is page directory
		uint64_t ignored2   : 4;
		uint64_t address	: 50;
		uint64_t reserved	: 1;
		uint64_t xd			: 1;  // if A32_EFER.NXE = 1, execute-disable -> 1-can't be executed, must be 0 if not enabled
	} 						flaggable;
	uint64_t 				number;
	uint64_t*				array;
} page_table_t;

typedef union page_directory {
	struct {
		uint64_t present	: 1;  // whether the page is present
		uint64_t rw			: 1;  // 0 = writes are not allowed, 1 = writers are allowed
		uint64_t us			: 1;  // 0 = CPL3 access is not allowed
		uint64_t pwt		: 1;  // page-level write-through
		uint64_t pcd		: 1;  // page-level cache disable
		uint64_t accessed	: 1;  // whether it was accessed
		uint64_t ignored	: 1;
		uint64_t ps			: 1;  // must be 0 since this is page directory
		uint64_t ignored2   : 4;
		uint64_t address	: 50;
		uint64_t reserved	: 1;
		uint64_t xd			: 1;  // if A32_EFER.NXE = 1, execute-disable -> 1-can't be executed, must be 0 if not enabled
	} 						flaggable;
	uint64_t 				number;
	uint64_t*				array;
} page_directory_t;

typedef union page_directory1GB {
	struct {
		uint64_t present	: 1;  // whether the page is present
		uint64_t rw			: 1;  // 0 = writes are not allowed, 1 = writers are allowed
		uint64_t us			: 1;  // 0 = CPL3 access is not allowed
		uint64_t pwt		: 1;  // page-level write-through
		uint64_t pcd		: 1;  // page-level cache disable
		uint64_t accessed	: 1;  // accessed
		uint64_t dirty		: 1;  // dirty
		uint64_t ps         : 1;  // must be 1
		uint64_t global		: 1;  // whether page is global or not
		uint64_t ignored	: 3;
		uint64_t pat		: 1;  // Indirectly determines the memory type used to access
		uint64_t ignored2   : 17;
		uint64_t address	: 24;
		uint64_t ignored3   : 6;
		uint64_t protectkey : 3;  // protection key
		uint64_t xd  		: 1;  // execute disable
	} 						flaggable;
	uint64_t 				number;
} page_directory1GB_t;

typedef union pdpt {
	struct {
		uint64_t present	: 1;  // whether the page is present
		uint64_t rw			: 1;  // 0 = writes are not allowed, 1 = writers are allowed
		uint64_t us			: 1;  // 0 = CPL3 access is not allowed
		uint64_t pwt		: 1;  // page-level write-through
		uint64_t pcd		: 1;  // page-level cache disable
		uint64_t reserved2  : 4;  // must be 0
		uint64_t ignored	: 3;
		uint64_t address	: 52;
		uint64_t reserved3  : 1;  // must be 0
	} 						flaggable;
	uint64_t 				number;
	uint64_t* 				array;
} pdpt_t;

typedef union pml4 {
	struct {
		uint64_t present	: 1;  // whether the page is present
		uint64_t rw			: 1;  // 0 = writes are not allowed, 1 = writers are allowed
		uint64_t us			: 1;  // 0 = CPL3 access is not allowed
		uint64_t pwt		: 1;  // page-level write-through
		uint64_t pcd		: 1;  // page-level cache disable
		uint64_t accessed	: 1;  // whether it was accessed
		uint64_t ignored	: 1;
		uint64_t ps			: 1;  // must be 0 since this is page directory
		uint64_t ignored2   : 4;
		uint64_t address	: 50;
		uint64_t reserved	: 1;
		uint64_t xd			: 1;  // if A32_EFER.NXE = 1, execute-disable -> 1-can't be executed, must be 0 if not enabled
	} 						flaggable;
	uint64_t 				number;
	uint64_t* 				array;
} pml4_t;

typedef union cr3_page_entry
{
   struct {
	   uint64_t ignored     : 2;   // ignored
	   uint64_t pwt         : 1;   // page-level write-through; indirectly determines the memory type used to access the PML4 table during linear-address translation (see Section 4.9.2)
	   uint64_t pcd		    : 1;   // page-level cache disable; indirectly determines the memory type used to access the PML4 table during linear-address translation (see Section 4.9.2)
	   uint64_t ignored2    : 7;   // ignored
	   uint64_t address     : 53;  // rest is address up to MAXPHYADDR, then zeros
   } 						flaggable;
   uint64_t 				number;
   uint64_t					pml;
} cr3_page_entry_t;

void initialize_paging(struct multiboot* mb);

uint64_t virtual_to_physical(uint64_t vaddress, uint8_t* valid);
uint64_t physical_to_virtual(uint64_t vaddress);

void allocate(uint64_t from, size_t amount, bool kernel, bool readonly);
void deallocate(uint64_t from, size_t amount);
bool allocated(uint64_t addr);
