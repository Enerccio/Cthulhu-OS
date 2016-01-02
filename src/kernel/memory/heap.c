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
 * INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A
 * PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
 * HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION
 * OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE
 * OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 *
 * heap.c
 *  Created on: Dec 22, 2015
 *      Author: Peter Vanusanik
 *  Contents: kernel heap memory organization
 */
#include "heap.h"

/**
 * temporary heap address is stored here, used initially to create memory structures
 * so standard allocation can happen
 */
puint_t tmp_heap;
/**
 * Start of the heap address
 */
uintptr_t heap_start_address;
/**
 * End of the heap address
 */
uintptr_t heap_end_address;

/**
 * Allocates the memory amount increased by align count, then aligns it and
 * returns it.
 */
aligned_ptr_t malign(size_t amount, uint16_t align) {
    if (tmp_heap != 0) {
        if (tmp_heap % align != 0) {
            tmp_heap = tmp_heap + (align - (tmp_heap % align));
        }
        puint_t head = tmp_heap;
        tmp_heap += amount;
        return (void*)(uintptr_t)head;
    }

    uintptr_t unaligned = (uintptr_t) malloc(amount+align+sizeof(uintptr_t));
    uintptr_t aligned = unaligned + sizeof(uintptr_t);
    if (!(aligned % align == 0)) {
        aligned = aligned + (align - (aligned % align));
    }
    uintptr_t* ptr = (uintptr_t*) aligned;
    *(ptr-1) = unaligned;
    return (void*)aligned;
}

void initialize_temporary_heap(puint_t temp_heap_start) {
    tmp_heap = temp_heap_start;
}

void initialize_standard_heap() {
    heap_end_address = heap_start_address = ADDRESS_OFFSET(RESERVED_KBLOCK_KHEAP_MAPPINGS);
    tmp_heap = 0;
}
