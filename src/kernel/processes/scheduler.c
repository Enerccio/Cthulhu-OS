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
#include <ds/llist.h>
#include <ds/array.h>
#include <ds/btree.h>

extern void wait_until_activated(ruint_t wait_code);
extern void proc_spinlock_lock(volatile void* memaddr);
extern void proc_spinlock_unlock(volatile void* memaddr);
extern void switch_to_usermode(ruint_t rdi, ruint_t rip, ruint_t rsp,
        ruint_t flags, ruint_t rsi, ruint_t rdx);
extern void* get_active_page();
extern void set_active_page(void* page);

#define INTERRUPT_FLAG (1<<9)

ruint_t __process_modifier;
ruint_t __thread_modifier;
ruint_t __halted_modifier;
bool    scheduler_enabled = false;

rb_tree_t*    halted_threads;
hash_table_t* mutex_waiters;
hash_table_t* mutex_status;
ruint_t       mutex_id;

uint64_t do_get_priority_count(cpu_t* cpu) {
	uint64_t count = 0;
	queue_t* queues[5] = { cpu->priority_0, cpu->priority_1, cpu->priority_2, cpu->priority_3, cpu->priority_4 };
	for (uint8_t i=0; i<5; i++) {
		count += queue_size(queues[i]) * (5-i);
	}
	return count;
}

uint64_t get_priority_count(cpu_t* cpu) {
    proc_spinlock_lock(&cpu->__cpu_lock);
    proc_spinlock_lock(&cpu->__cpu_sched_lock);

    // TODO:
    uint64_t count = do_get_priority_count(cpu);

    proc_spinlock_unlock(&cpu->__cpu_lock);
    proc_spinlock_unlock(&cpu->__cpu_sched_lock);

    return count;
}

void attemp_to_run_scheduler(registers_t* r) {
    rg_t rd = rg_create_random_generator(get_unix_time());
    uint32_t ticks = 0;

    do {
        cpu_t* cpu = array_get_random(cpus, &rd);
        if (do_get_priority_count(cpu) > 0) {
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
    t->last_rflags = r->rflags;
}

void registers_copy(thread_t* t, registers_t* r) {
    r->rip = t->last_rip;
    r->uesp = t->last_rsp;
    r->rbp = t->last_rbp;
    r->rax = t->last_rax;
    r->rbx = t->last_rbx;
    r->rcx = t->last_rcx;
    r->rdx = t->last_rdx;
    r->rsi = t->last_rsi;
    r->rdi = t->last_rdi;
    r->r8 = t->last_r8;
    r->r9 = t->last_r9;
    r->r10 = t->last_r10;
    r->r11 = t->last_r11;
    r->r12 = t->last_r12;
    r->r13 = t->last_r13;
    r->r14 = t->last_r14;
    r->r15 = t->last_r15;
    r->rflags = t->last_rflags;
}

thread_t* get_and_update_queue(cpu_t* cpu, queue_t* selq, uint8_t ord, queue_t** queues) {
	thread_t* winner = queue_pop(selq);
	queue_t* popq = NULL;
	queue_t* pushq = selq;
	for (uint8_t i=ord+1; i<5; i++) {
		queue_t* tq = queues[i];
		if (queue_has_elements(tq)) {
			popq = tq;
			break;
		}
		pushq = tq;
	}
	if (popq != NULL) {
		thread_t* ptpick = queue_pop(popq);
		queue_push(pushq, ptpick);
	}
	return winner;
}

thread_t* get_and_update_threads(cpu_t* cpu, queue_t** queues) {
	for (uint8_t i=0; i<5; i++) {
		queue_t* tq = queues[i];
		if (queue_has_elements(tq)) {
			return get_and_update_queue(cpu, tq, i, queues);
		}
	}
	return NULL;
}

void context_switch(registers_t* r, cpu_t* cpu, thread_t* old_head, thread_t* selection) {
	if (old_head != NULL && !old_head->blocked) {
		queue_t* queues[5] = { cpu->priority_0, cpu->priority_1, cpu->priority_2, cpu->priority_3, cpu->priority_4 };
		queue_push(queues[old_head->priority], old_head);
	}

	if (old_head != selection) {
		cpu->ct = selection;
		if (r != NULL && r->cs != 8) {
			copy_registers(r, old_head);
		}

	} else if (r != NULL && r->cs == (24|0x0003)) {
		proc_spinlock_unlock(&__thread_modifier);
		proc_spinlock_unlock(&cpu->__cpu_lock);
		proc_spinlock_unlock(&cpu->__cpu_sched_lock);
		return; // same thread
	}

	uintptr_t pml4 = (uintptr_t)get_active_page();
	if (cpu->ct->parent_process->pml4 != pml4) {
		set_active_page((void*)cpu->ct->parent_process->pml4);
	}

	pml4 = (uintptr_t)get_active_page();
	__atomic_store_n(&cpu->current_address_space, pml4, __ATOMIC_SEQ_CST);

	if (r != NULL) {
		r->cs = 24 | 0x0003; // user space code
		r->ss = 32 | 0x0003; // user space data
		// TODO: add thread locals
		r->ds = 32 | 0x0003; // user space data
		r->es = 32 | 0x0003; // user space data

		registers_copy(cpu->ct, r);
	}

	proc_spinlock_unlock(&__thread_modifier);
	proc_spinlock_unlock(&cpu->__cpu_lock);
	proc_spinlock_unlock(&cpu->__cpu_sched_lock);

	if (r == NULL) {
		// TODO: add flags for io
		ruint_t flags = INTERRUPT_FLAG;
		switch_to_usermode(cpu->ct->last_rdi,
				cpu->ct->last_rip, cpu->ct->last_rsp, flags,
				cpu->ct->last_rsi,
				cpu->ct->last_rcx);
	}
}

// TODO add switching threads
void schedule(registers_t* r) {
    cpu_t* cpu = get_current_cput();

    proc_spinlock_lock(&cpu->__cpu_lock);
    proc_spinlock_lock(&cpu->__cpu_sched_lock);

    while (do_get_priority_count(cpu) == 0) {
        r = NULL; // discard remaining stack info, we won't be jumping from this
        proc_spinlock_unlock(&cpu->__cpu_lock);
        proc_spinlock_unlock(&cpu->__cpu_sched_lock);
        ENABLE_INTERRUPTS();
        wait_until_activated(WAIT_SCHEDULER_QUEUE_CHNG);
        proc_spinlock_lock(&cpu->__cpu_lock);
        proc_spinlock_lock(&cpu->__cpu_sched_lock);
    }

    proc_spinlock_lock(&__thread_modifier);

    queue_t* queues[5] = { cpu->priority_0, cpu->priority_1, cpu->priority_2, cpu->priority_3, cpu->priority_4 };
    thread_t* selection = get_and_update_threads(cpu, queues);
    thread_t* old_head = cpu->ct;

    context_switch(r, cpu, old_head, selection);
}

void enschedule(thread_t* t, cpu_t* cpu) {
    proc_spinlock_lock(&cpu->__cpu_lock);
    proc_spinlock_lock(&cpu->__cpu_sched_lock);
    proc_spinlock_lock(&__thread_modifier);

    queue_t* queues[5] = { cpu->priority_0, cpu->priority_1, cpu->priority_2, cpu->priority_3, cpu->priority_4 };
	queue_push(queues[t->priority], t);

    if (cpu != get_current_cput()) {
        send_ipi_nowait(cpu->apic_id, IPI_RUN_SCHEDULER, 0, 0, NULL);
    }

    proc_spinlock_unlock(&__thread_modifier);
    proc_spinlock_unlock(&cpu->__cpu_sched_lock);
    proc_spinlock_unlock(&cpu->__cpu_lock);
}

void enschedule_to_self(thread_t* t) {
    enschedule(t, get_current_cput());
}

void enschedule_best(thread_t* t) {
    cpu_t* mincpu = array_get_at(cpus, 0);
    uint64_t priority = get_priority_count(mincpu);
    for (uint32_t i=1; i<array_get_size(cpus); i++) {
        cpu_t* test = array_get_at(cpus, 0);
        uint64_t np = get_priority_count(test);
        if (priority > np) {
            mincpu = test;
            priority = np;
        }
    }

    enschedule(t, mincpu);
}

__attribute__((hot)) static int __cmp_tid(const void* a, const void* b) {
	tid_t ta = (tid_t)(uintptr_t)a;
	tid_t tb = (tid_t)(uintptr_t)b;
	if (ta > tb)
		return -1;
	else if (ta < tb)
		return 1;
	else
		return 0;
}

static void __nop(void* ignored) {

}

void initialize_scheduler() {
    __process_modifier = 0;
    __thread_modifier = 0;
    __halted_modifier = 0;

    halted_threads = rb_create_tree(__cmp_tid, __nop, __nop);
    mutex_waiters = create_uint64_table();
    mutex_status = create_uint64_table();
    mutex_id = 0;
}

uint64_t new_mutex() {
	cpu_t* cpu = get_current_cput();

	proc_spinlock_lock(&cpu->__cpu_lock);
	proc_spinlock_lock(&cpu->__cpu_sched_lock);
	proc_spinlock_lock(&__thread_modifier);
	proc_spinlock_lock(&__halted_modifier);
	uint64_t mtxid = mutex_id++;
	table_set(mutex_waiters, (void*)mtxid, create_array());
	table_set(mutex_status, (void*)mtxid, (void*)false);
	list_push_right(cpu->ct->parent_process->mutexes, (void*)mtxid);
	proc_spinlock_unlock(&__halted_modifier);
	proc_spinlock_unlock(&__thread_modifier);
	proc_spinlock_unlock(&cpu->__cpu_sched_lock);
	proc_spinlock_unlock(&cpu->__cpu_lock);

	return mtxid;
}

void unblock_mutex_waits(uint64_t mtxid) {
	proc_spinlock_lock(&__halted_modifier);
	if (!table_contains(mutex_waiters, (void*)mtxid)){
		proc_spinlock_unlock(&__thread_modifier);
		return;
	}

	array_t* wtarray = (array_t*)table_get(mutex_waiters, (void*)mtxid);
	for (uint32_t i=0; i<array_get_size(wtarray); i++) {
		tid_t bttid = (tid_t)(uintptr_t) array_get_at(wtarray, i);
		rb_node_t* bnode = tree_find(halted_threads, (void*)bttid);
		thread_t* blocked_thread = (thread_t*) bnode->info;
		blocked_thread->blocked = false;
		rb_delete(halted_threads, bnode);
		enschedule_best(blocked_thread);
	}
	array_clean(wtarray);
	table_set(mutex_status, (void*)mtxid, (void*)false);

	proc_spinlock_unlock(&__halted_modifier);
}

void block_mutex_waits(uint64_t mtxid) {
	proc_spinlock_lock(&__halted_modifier);
	if (!table_contains(mutex_waiters, (void*)mtxid)){
		proc_spinlock_unlock(&__thread_modifier);
		return;
	}

	table_set(mutex_status, (void*)mtxid, (void*)true);

	proc_spinlock_unlock(&__halted_modifier);
}

void block_wait_mutex(uint64_t mtxid, registers_t* registers) {
	cpu_t* cpu = get_current_cput();

	proc_spinlock_lock(&cpu->__cpu_lock);
	proc_spinlock_lock(&cpu->__cpu_sched_lock);
	proc_spinlock_lock(&__thread_modifier);
	proc_spinlock_lock(&__halted_modifier);

	if (!table_contains(mutex_waiters, (void*)mtxid)){
		proc_spinlock_unlock(&__thread_modifier);
		proc_spinlock_unlock(&cpu->__cpu_sched_lock);
		proc_spinlock_unlock(&cpu->__cpu_lock);
		proc_spinlock_unlock(&__thread_modifier);
		return;
	}

	if (!((bool)table_get(mutex_status, (void*)mtxid))) {
		proc_spinlock_unlock(&__thread_modifier);
		proc_spinlock_unlock(&cpu->__cpu_sched_lock);
		proc_spinlock_unlock(&cpu->__cpu_lock);
		proc_spinlock_unlock(&__halted_modifier);
		return;
	}

	thread_t* thread = cpu->ct;
	thread->blocked = true;
	array_t* wtarray = (array_t*)table_get(mutex_waiters, (void*)mtxid);
	array_push_data(wtarray, (void*)thread->tId);
	rb_insert(halted_threads, (void*)thread->tId, thread);

	proc_spinlock_unlock(&__thread_modifier);
	proc_spinlock_unlock(&cpu->__cpu_sched_lock);
	proc_spinlock_unlock(&cpu->__cpu_lock);
	proc_spinlock_unlock(&__halted_modifier);

	schedule(registers);
}
