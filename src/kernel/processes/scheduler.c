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
 * scheduler.c
 *  Created on: Jan 1, 2016
 *      Author: Peter Vanusanik
 *  Contents: 
 */

#include "scheduler.h"
#include "../cpus/cpu_mgmt.h"

#include <stdnoreturn.h>

extern void wait_until_activated(uint64_t wait_code);
extern void proc_spinlock_lock(volatile void* memaddr);
extern void proc_spinlock_unlock(volatile void* memaddr);
extern noreturn switch_to_usermode(uint64_t rdat, uint64_t rip, uint64_t rsp, uint64_t flags);
extern void* get_active_page();
extern void set_active_page(void* page);

#define INTERRUPT_FLAG (1<<9)

uint64_t __process_modifier;
uint64_t __thread_modifier;

// TODO add switching threads
// TODO add borrowing/returning stacks
void schedule(registers_t* r) {
	cpu_t* cpu = get_current_cput();

	proc_spinlock_lock(&cpu->__cpu_lock);
	proc_spinlock_lock(&cpu->__cpu_sched_lock);

	while (cpu->threads == NULL) {
		r = NULL; // discard remaining stack info, we won't be jumping from this
		proc_spinlock_unlock(&cpu->__cpu_lock);
		proc_spinlock_unlock(&cpu->__cpu_sched_lock);
		wait_until_activated(WAIT_SCHEDULER_QUEUE_CHNG);
		proc_spinlock_lock(&cpu->__cpu_lock);
		proc_spinlock_lock(&cpu->__cpu_sched_lock);
	}

	proc_spinlock_lock(&__thread_modifier);

	rg_t schedule_random = rg_create_random_generator(get_unix_time()*get_unix_time_ms());
	uint64_t winner = rg_next_uint_l(&schedule_random, cpu->total_tickets);

	thread_t* selection = cpu->threads;
	uint64_t csel = 0;
	while (selection->next_thread != NULL) {
		if (selection->tickets + csel > winner && csel <= winner) {
			// we have winner
			break;
		}
	}

	thread_t* old_head = cpu->threads;
	if (old_head != selection) {
		thread_t* tmp;
		tmp=old_head->next_thread;
		old_head->next_thread = selection->next_thread;
		selection->next_thread = tmp;
		old_head->prev_thread = selection->prev_thread;
		selection->prev_thread = NULL;
		cpu->threads = selection;
		if (r != NULL && r->cs != 8) {
			old_head->last_rip = r->rip;
			old_head->last_rsp = r->uesp;
			old_head->last_rax = r->rax;
			old_head->last_rcx = r->rcx;
			old_head->last_rdx = r->rdx;
			old_head->last_rdi = r->rdi;
			old_head->last_rsi = r->rsi;
			old_head->last_r8 = r->r8;
			old_head->last_r9 = r->r9;
			old_head->last_r10 = r->r10;
			old_head->last_r11 = r->r11;
			old_head->last_rbp = r->rbp;
			old_head->last_rbx = r->rbx;
			old_head->last_r12 = r->r12;
			old_head->last_r13 = r->r13;
			old_head->last_r14 = r->r14;
			old_head->last_r15 = r->r15;
		}
	}

	// TODO: add flags for io
	uint64_t flags = INTERRUPT_FLAG;

	if (r != NULL) {
		if (cpu->threads->first_call) {
			r->rdi = (uint64_t)cpu->threads->initial_thread_data;
		}
		r->cs = 24; // user space code
		r->ss = 32; // user space data
		// TODO: add thread locals
		r->ds = 32; // user space data
		r->es = 32; // user space data
		r->rflags = flags;

		r->rip = cpu->threads->last_rip;
		r->uesp = cpu->threads->last_rsp;
		r->rbp = cpu->threads->last_rbp;
		r->rax = cpu->threads->last_rax;
		r->rbx = cpu->threads->last_rbx;
		r->rcx = cpu->threads->last_rcx;
		r->rdx = cpu->threads->last_rdx;
		r->rsi = cpu->threads->last_rsi;
		r->rdi = cpu->threads->last_rdi;
		r->r8 = cpu->threads->last_r8;
		r->r9 = cpu->threads->last_r9;
		r->r10 = cpu->threads->last_r10;
		r->r11 = cpu->threads->last_r11;
		r->r12 = cpu->threads->last_r12;
		r->r13 = cpu->threads->last_r13;
		r->r14 = cpu->threads->last_r14;
		r->r15 = cpu->threads->last_r15;
	}

	uint64_t pml4 = (uint64_t)get_active_page();
	if (cpu->threads->parent_process->pml4 != pml4) {
		set_active_page((void*)cpu->threads->parent_process->pml4);
	}

	proc_spinlock_unlock(&__thread_modifier);
	proc_spinlock_unlock(&cpu->__cpu_lock);
	proc_spinlock_unlock(&cpu->__cpu_sched_lock);

	if (r == NULL) {
		switch_to_usermode((uint64_t)cpu->threads->initial_thread_data,
				cpu->threads->last_rip, cpu->threads->last_rsp, flags);
	}
}

void initialize_scheduler() {
	__process_modifier = 0;
	__thread_modifier = 0;
}
