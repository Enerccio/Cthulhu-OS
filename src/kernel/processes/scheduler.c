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
#include "../cpus/ipi.h"

#include <stdnoreturn.h>
#include <ds/hmap.h>

extern void wait_until_activated(ruint_t wait_code);
extern void proc_spinlock_lock(volatile void* memaddr);
extern void proc_spinlock_unlock(volatile void* memaddr);
extern noreturn void switch_to_usermode(ruint_t rdi, ruint_t rip, ruint_t rsp,
		ruint_t flags, ruint_t rsi, ruint_t rdx);
extern void* get_active_page();
extern void set_active_page(void* page);

#define INTERRUPT_FLAG (1<<9)

ruint_t __process_modifier;
ruint_t __thread_modifier;
bool    scheduler_enabled = false;

void attemp_to_run_scheduler(registers_t* r) {
    rg_t rd = rg_create_random_generator(get_unix_time());
    uint32_t ticks = 0;

    do {
        cpu_t* cpu = array_get_random(cpus, &rd);
        if (cpu->threads != NULL) {
        	if (cpu == get_current_cput()) {
        		if (cpu->__cpu_lock == 1)
        			continue;
        	}
            send_ipi_nowait(cpu->apic_id, IPI_RUN_SCHEDULER, 0, 0, r);
            break;
        }
        ++ticks;
    } while (ticks < array_get_size(cpus));
}

void release_tickets(thread_t* t) {
	borrowed_ticket_t* bt;
	list_iterator_t li;
	list_create_iterator(t->borrowed_tickets, &li);
	while (list_has_next(&li)) {
		bt = list_next(&li);
		if (bt->source == NULL) {
			// source no longer exist
			free(bt);
		} else {
			if (bt->release_now && t->tickets>=bt->tamount) {
				bt->source->tickets += bt->tamount;
				t->tickets -= bt->tamount;

				list_remove(bt, bt->source->lended_tickets);

				list_remove_it(&li);
				free(bt);
			}
		}
	}
}

void copy_registers(registers_t* r, thread_t* t) {
	t->last_rip = r->rip;
	t->last_rsp = r->uesp;
	t->last_rax = r->rax;
	t->last_rcx = r->rcx;
	t->last_rdx = r->rdx;
	t->last_rdi = r->rdi;
	t->last_rsi = r->rsi;
	t->last_r8  = r->r8;
	t->last_r9  = r->r9;
	t->last_r10 = r->r10;
	t->last_r11 = r->r11;
	t->last_rbp = r->rbp;
	t->last_rbx = r->rbx;
	t->last_r12 = r->r12;
	t->last_r13 = r->r13;
	t->last_r14 = r->r14;
	t->last_r15 = r->r15;
}

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

    DISABLE_INTERRUPTS();
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
        csel += selection->tickets;
        selection = selection->next_thread;
    }

    thread_t* old_head = cpu->threads;
    if (old_head != selection) {
        selection->prev_thread->next_thread = selection->next_thread;
        if (selection->next_thread != NULL) {
        	selection->next_thread->prev_thread = selection->prev_thread;
        }

        selection->next_thread = old_head;
        selection->prev_thread = NULL;
        old_head->prev_thread = selection;

        cpu->threads = selection;
        if (r != NULL && r->cs != 8) {
            copy_registers(r, old_head);
        }

        release_tickets(old_head);

    } else if (r != NULL && r->cs == (24|0x0003)){
    	proc_spinlock_unlock(&__thread_modifier);
		proc_spinlock_unlock(&cpu->__cpu_lock);
		proc_spinlock_unlock(&cpu->__cpu_sched_lock);
    	return; // same thread
    }

    // TODO: add flags for io
    ruint_t flags = INTERRUPT_FLAG;

    if (r != NULL) {
        r->cs = 24 | 0x0003; // user space code
        r->ss = 32 | 0x0003; // user space data
        // TODO: add thread locals
        r->ds = 32 | 0x0003; // user space data
        r->es = 32 | 0x0003; // user space data
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

    uintptr_t pml4 = (uintptr_t)get_active_page();
    if (cpu->threads->parent_process->pml4 != pml4) {
        set_active_page((void*)cpu->threads->parent_process->pml4);
    }

    proc_spinlock_unlock(&__thread_modifier);
    proc_spinlock_unlock(&cpu->__cpu_lock);
    proc_spinlock_unlock(&cpu->__cpu_sched_lock);

    if (r == NULL) {
        switch_to_usermode(cpu->threads->last_rdi,
                cpu->threads->last_rip, cpu->threads->last_rsp, flags,
				cpu->threads->last_rsi,
				cpu->threads->last_rcx);
    }
}

void enschedule(thread_t* t, cpu_t* cpu) {
	proc_spinlock_lock(&cpu->__cpu_lock);
	proc_spinlock_lock(&cpu->__cpu_sched_lock);
	proc_spinlock_lock(&__thread_modifier);

	if (cpu->threads == NULL) {
		cpu->threads = t;
		t->next_thread = NULL;
		t->prev_thread = NULL;
	} else {
		thread_t* nt = cpu->threads->next_thread;
		if (nt == NULL) {
			t->next_thread = NULL;
			cpu->threads->next_thread = t;
		} else {
			t->next_thread = cpu->threads->next_thread;
			cpu->threads->next_thread->prev_thread = t;
		}
		t->prev_thread = cpu->threads;
		cpu->threads->next_thread = t;
	}

	cpu->total_tickets += t->tickets;

	if (cpu != get_current_cput()) {
		send_ipi_nowait(cpu->apic_id, IPI_RUN_SCHEDULER, 0, 0, NULL);
	}

	proc_spinlock_unlock(&__thread_modifier);
	proc_spinlock_unlock(&cpu->__cpu_lock);
	proc_spinlock_unlock(&cpu->__cpu_sched_lock);
}

void enschedule_to_self(thread_t* t) {
	enschedule(t, get_current_cput());
}

uint64_t get_tickets_from(cpu_t* cpu) {
	proc_spinlock_lock(&cpu->__cpu_lock);
	proc_spinlock_lock(&cpu->__cpu_sched_lock);
	proc_spinlock_lock(&__thread_modifier);

	uint64_t tcount = 0;
	thread_t* t = cpu->threads;
	while (t != NULL) {
		tcount += t->tickets;
		t = t->next_thread;
	}

	proc_spinlock_unlock(&__thread_modifier);
	proc_spinlock_unlock(&cpu->__cpu_lock);
	proc_spinlock_unlock(&cpu->__cpu_sched_lock);

	return tcount;
}

void enschedule_best(thread_t* t) {
	cpu_t* mincpu = array_get_at(cpus, 0);
	uint64_t tickets = get_tickets_from(mincpu);
	for (uint32_t i=1; i<array_get_size(cpus); i++) {
		cpu_t* test = array_get_at(cpus, 0);
		uint64_t nt = get_tickets_from(test);
		if (tickets > nt) {
			mincpu = test;
			tickets = nt;
		}
	}

	enschedule(t, mincpu);
}

void initialize_scheduler() {
    __process_modifier = 0;
    __thread_modifier = 0;
}
