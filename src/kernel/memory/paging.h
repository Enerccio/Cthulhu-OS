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
        uint64_t present    : 1;  // whether the page is present
        uint64_t rw         : 1;  // 0 = writes are not allowed, 1 = writers are allowed
        uint64_t us         : 1;  // 0 = CPL3 access is not allowed
        uint64_t pwt        : 1;  // page-level write-through
        uint64_t pcd        : 1;  // page-level cache disable
        uint64_t accessed   : 1;  // whether it was accessed
        uint64_t dirty      : 1;  // whether it was written to
        uint64_t pat        : 1;  // must be 0 unless PAT is supported
        uint64_t global     : 1;  // global if PGE=1, ignored otherwise
        uint64_t ignored    : 3;
        uint64_t address    : 50; // must be 0x1000 aligned
        uint64_t reserved   : 1;
        uint64_t xd         : 1;  // if A32_EFER.NXE = 1, execute-disable -> 1-can't be executed, must be 0 if not enabled
    }                       flaggable;
    struct {
        uint64_t copy        : 9;
        uint64_t do_not_copy : 54;
        uint64_t copy2       : 1;
    }                       copyinfo;
    uint64_t                address;
} page_t;

typedef union page_table {
    struct {
        uint64_t present    : 1;  // whether the page is present
        uint64_t rw         : 1;  // 0 = writes are not allowed, 1 = writers are allowed
        uint64_t us         : 1;  // 0 = CPL3 access is not allowed
        uint64_t pwt        : 1;  // page-level write-through
        uint64_t pcd        : 1;  // page-level cache disable
        uint64_t accessed   : 1;  // whether it was accessed
        uint64_t ignored    : 1;
        uint64_t ps         : 1;  // must be 0 since this is page directory
        uint64_t ignored2   : 4;
        uint64_t address    : 50;
        uint64_t reserved   : 1;
        uint64_t xd         : 1;  // if A32_EFER.NXE = 1, execute-disable -> 1-can't be executed, must be 0 if not enabled
    }                       flaggable;
    struct {
       uint64_t copy        : 7;
       uint64_t do_not_copy : 56;
       uint64_t copy2       : 1;
    }                       copyinfo;
    uint64_t                number;
    uint64_t*               array;
} page_table_t;

typedef union page_directory {
    struct {
        uint64_t present    : 1;  // whether the page is present
        uint64_t rw         : 1;  // 0 = writes are not allowed, 1 = writers are allowed
        uint64_t us         : 1;  // 0 = CPL3 access is not allowed
        uint64_t pwt        : 1;  // page-level write-through
        uint64_t pcd        : 1;  // page-level cache disable
        uint64_t accessed   : 1;  // whether it was accessed
        uint64_t ignored    : 1;
        uint64_t ps         : 1;  // must be 0 since this is page directory
        uint64_t ignored2   : 4;
        uint64_t address    : 50;
        uint64_t reserved   : 1;
        uint64_t xd         : 1;  // if A32_EFER.NXE = 1, execute-disable -> 1-can't be executed, must be 0 if not enabled
    }                       flaggable;
    struct {
       uint64_t copy        : 8;
       uint64_t do_not_copy : 55;
       uint64_t copy2       : 1;
    }                       copyinfo;
    uint64_t                number;
    uint64_t*               array;
} page_directory_t;

typedef union page_directory1GB {
    struct {
        uint64_t present    : 1;  // whether the page is present
        uint64_t rw         : 1;  // 0 = writes are not allowed, 1 = writers are allowed
        uint64_t us         : 1;  // 0 = CPL3 access is not allowed
        uint64_t pwt        : 1;  // page-level write-through
        uint64_t pcd        : 1;  // page-level cache disable
        uint64_t accessed   : 1;  // accessed
        uint64_t dirty      : 1;  // dirty
        uint64_t ps         : 1;  // must be 1
        uint64_t global     : 1;  // whether page is global or not
        uint64_t ignored    : 3;
        uint64_t pat        : 1;  // Indirectly determines the memory type used to access
        uint64_t ignored2   : 17;
        uint64_t address    : 24;
        uint64_t ignored3   : 6;
        uint64_t protectkey : 3;  // protection key
        uint64_t xd         : 1;  // execute disable
    }                       flaggable;
    struct {
       uint64_t copy        : 13;
       uint64_t do_not_copy : 50;
       uint64_t copy2       : 1;
    }                       copyinfo;
    uint64_t                number;
} page_directory1GB_t;

typedef union pdpt {
    struct {
        uint64_t present    : 1;  // whether the page is present
        uint64_t rw         : 1;  // 0 = writes are not allowed, 1 = writers are allowed
        uint64_t us         : 1;  // 0 = CPL3 access is not allowed
        uint64_t pwt        : 1;  // page-level write-through
        uint64_t pcd        : 1;  // page-level cache disable
        uint64_t reserved2  : 4;  // must be 0
        uint64_t ignored    : 3;
        uint64_t address    : 52;
        uint64_t reserved3  : 1;  // must be 0
    }                       flaggable;
    struct {
       uint64_t copy        : 5;
       uint64_t do_not_copy : 59;
    }                       copyinfo;
    uint64_t                number;
    uint64_t*               array;
} pdpt_t;

typedef union cr3_page_entry
{
   struct {
       uint64_t ignored     : 2;   // ignored
       uint64_t pwt         : 1;   // page-level write-through; indirectly determines the memory type used to access the PML4 table during linear-address translation (see Section 4.9.2)
       uint64_t pcd         : 1;   // page-level cache disable; indirectly determines the memory type used to access the PML4 table during linear-address translation (see Section 4.9.2)
       uint64_t ignored2    : 7;   // ignored
       uint64_t address     : 53;  // rest is address up to MAXPHYADDR, then zeros
   }                        flaggable;
   struct {
       uint64_t copy        : 4;
       uint64_t do_not_copy : 60;
   }                        copyinfo;
   uint64_t                 number;
   uint64_t                 pml;
} cr3_page_entry_t;

typedef struct stack_element {
    uint64_t frame_address;
    uint64_t array_ord;
    struct stack_element* next;
} stack_element_t;

typedef struct frame_info frame_info_t;

typedef struct section_info {
    stack_element_t* head;
    struct section_info* next_section;
    frame_info_t* frame_array;

    uint64_t start_word;
    uint64_t end_word;
    uint64_t total_frames;
} section_info_t;

struct frame_info {
    uint32_t usage_count;
    uint32_t cow_count;
    stack_element_t* bound_stack_element;
};

/**
 * Initializes paging subsystem with correct paging, requiring multiboot
 * for memory info.
 */
void initialize_physical_memory_allocation(struct multiboot_info* mb);

/**
 * Translates virtual address to physical. valid pointer will
 * contain true if address is translated to valid one.
 */
puint_t virtual_to_physical(uint64_t vaddress, uint8_t* valid);
/**
 * Translates physical address to virtual using memory identity map.
 */
uint64_t physical_to_virtual(puint_t paddress);

/**
 * Allocates memory from specified address and amount, using
 * kernel and readonly as source flags.
 */
void allocate(uintptr_t from, size_t amount, bool kernel, bool readonly);
/**
 * Deallocates memory from specified address and amount.
 */
void deallocate(uintptr_t from, size_t amount);
/**
 * Returns whether specific virtual address is allocated or not.
 */
bool allocated(uintptr_t addr);

/**
 * Deallocates starting memory.
 */
void deallocate_start_memory();

/**
 * Dealocates starting address at point, used to deallocate modules
 */
void deallocate_starting_address(uintptr_t address, size_t size);

puint_t clone_paging_structures();

bool page_fault(uintptr_t address, ruint_t errcode);

void allocate_physret(uintptr_t block_addr, puint_t* physmem, bool kernel, bool rw);

#define CHNG_TYPE_RW 0
#define CHNG_TYPE_SU 1

void mem_change_type(uintptr_t start, size_t len, int change_type, bool new_value, bool invalidate_others);

void map_range(uintptr_t start, uintptr_t end, uintptr_t tostart, uintptr_t fromstart, bool virtual_memory,
		bool readonly, bool kernel);
