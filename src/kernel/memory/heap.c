/*
 * The MIT License (MIT)
 * Copyright (c) 2015 Peter Vanusanik <admin@en-circle.com>
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 *
 * heap.c
 *  Created on: Dec 22, 2015
 *      Author: Peter Vanusanik
 *  Contents: kernel heap memory organization
 */
#include "heap.h"

int64_t tmp_heap;
uint64_t heap_start_address;
uint64_t heap_end_address;

uint64_t handle_kernel_memory(int required_amount){
	if (required_amount > 0){
		uint64_t old_heap_end = heap_end_address;
		heap_end_address += required_amount;
		allocate(old_heap_end, required_amount, true, false);
		return old_heap_end;
	} else {
		required_amount = 0-required_amount;
		uint64_t old_heap_end = heap_end_address;
		uint64_t new_heap_end = heap_end_address - required_amount;
		if (new_heap_end < heap_start_address){
			required_amount = heap_end_address - heap_start_address;
			new_heap_end = heap_start_address;
		}
		heap_end_address = new_heap_end;
		deallocate(heap_end_address, required_amount);
		return old_heap_end;
	}
}

aligned_ptr_t malign(size_t amount, uint16_t align){
	if (tmp_heap != 0){
		if (tmp_heap % align != 0){
			tmp_heap = tmp_heap + (align - (tmp_heap % align));
		}
		uint64_t head = tmp_heap;
		tmp_heap += amount;
		return (void*)head;
	}

	uint64_t unaligned = (uint64_t) malloc(amount+align+sizeof(uint64_t));
	uint64_t aligned = unaligned + sizeof(uint64_t);
	if (!(aligned % align == 0)){
		aligned = aligned + (align - (aligned % align));
	}
	uint64_t* ptr = (uint64_t*) aligned;
	*(ptr-1) = unaligned;
	return (void*)aligned;
}

void initialize_temporary_heap(uint64_t temp_heap_start){
	tmp_heap = temp_heap_start;
}

void initialize_standard_heap(){
	heap_end_address = heap_start_address = ADDRESS_OFFSET(RESERVED_KBLOCK_KHEAP_MAPPINGS);
	tmp_heap = 0;
}
