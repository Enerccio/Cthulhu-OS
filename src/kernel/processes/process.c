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
 * task.c
 *  Created on: Dec 27, 2015
 *      Author: Peter Vanusanik
 *  Contents: 
 */

#include "process.h"

#include "../interrupts/clock.h"

#include <stdatomic.h>

ruint_t __proclist_lock;
ruint_t process_id_num;
ruint_t thread_id_num;
array_t* processes;

extern void proc_spinlock_lock(volatile void* memaddr);
extern void proc_spinlock_unlock(volatile void* memaddr);

// TODO: add memory failure checks
proc_t* create_process_structure(uintptr_t pml) {
	proc_t* process = malloc(sizeof(proc_t));

	proc_spinlock_lock(&__proclist_lock);
	process->proc_id = ++process_id_num;
	array_push_data(processes, process);
	proc_spinlock_unlock(&__proclist_lock);

	process->fds = create_array();
	process->threads = create_array();
	process->pml4 = pml;
	process->mem_maps = NULL;
	process->proc_random = rg_create_random_generator(get_unix_time());

	thread_t* main_thread = malloc(sizeof(thread_t));
	memset(main_thread, 0, sizeof(thread_t));
	main_thread->parent_process = process;
	main_thread->tickets = PER_PROCESS_TICKETS;
	main_thread->tId = __atomic_add_fetch(&thread_id_num, 1, __ATOMIC_SEQ_CST);
	array_push_data(process->threads, main_thread);

    return process;
}

void initialize_processes() {
	__proclist_lock = 0;
	process_id_num = 0;
	thread_id_num = 0;
	processes = create_array_spec(256);
}

mmap_area_t* request_va_hole(proc_t* proc, uintptr_t start_address, size_t req_size) {
	mmap_area_t** lm = &proc->mem_maps;
	uintptr_t x1, x2, y1, y2;

	while (*lm != NULL) {
		mmap_area_t* mmap = *lm;

		x1 = start_address;
		x2 = start_address + req_size;
		y1 = mmap->vastart;
		y2 = mmap->vaend;

		if (start_address >= mmap->vaend) {
			lm = &mmap->next;
			continue;
		} else if (x1 <= y2 && y1 <= x2) {
			// section overlaps
			return NULL;
		} else {
			// we have found the section above, therefore we are done with the search and we need to
			// insert
			break;
		}
	}

	mmap_area_t* newmm = malloc(sizeof(mmap_area_t));
	memset(newmm, 0, sizeof(mmap_area_t));
	newmm->next = *lm;
	newmm->vastart = start_address;
	newmm->vaend = start_address + req_size;
	*lm = newmm;
	return newmm;
}

#define ALIGN_DOWN(a, b) ((a) - (a % b))
#define ALIGN_UP(a, b) ((a % b == 0) ? (a) : (ALIGN_DOWN(a, b) + b))

mmap_area_t* find_va_hole(proc_t* proc, size_t req_size, size_t align_amount) {
	mmap_area_t** lm = &proc->mem_maps;
	uintptr_t start_address = 0;
	uintptr_t hole = 0;

	if (*lm == NULL)
		hole = 0x800000000000;

	while (*lm != NULL) {
		mmap_area_t* mmap = *lm;

		hole = mmap->vastart - start_address;
		if (hole >= req_size+align_amount)
			break; // hole found

		start_address = mmap->vaend;
		lm = &mmap->next;
		if (*lm == NULL) {
			hole = 0x800000000000 - start_address;
		}
	}

	start_address = ALIGN_UP(start_address, align_amount);
	size_t diff_holes = hole - req_size;
	uintptr_t offset;
	if (diff_holes == 0)
		offset = 0;
	else
		offset = rg_next_uint_l(&proc->proc_random, diff_holes);
	offset = ALIGN_DOWN(offset, align_amount);
	mmap_area_t* newmm = malloc(sizeof(mmap_area_t));
	memset(newmm, 0, sizeof(mmap_area_t));
	newmm->next = *lm;
	newmm->vastart = start_address + offset;
	newmm->vaend = start_address + req_size + offset;
	*lm = newmm;
	return newmm;
}
