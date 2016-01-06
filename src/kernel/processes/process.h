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
 * task.h
 *  Created on: Dec 27, 2015
 *      Author: Peter Vanusanik
 *  Contents: 
 */

#pragma once

#include <sys/types.h>
#include "../commons.h"
#include "../interrupts/idt.h"

#include <ds/array.h>
#include <ds/random.h>
#include <ds/llist.h>

typedef struct file_descriptor {
    uint32_t fd_pid;
    uint64_t owner_daemon;
} fd_t;

typedef enum mmap_area_type {
    program_data, stack_data, heap_data
} ma_type_t;

typedef struct mmap_area {
    uintptr_t  vastart;
    uintptr_t  vaend;
    ma_type_t mtype;

    struct mmap_area* next;
} mmap_area_t;

typedef struct thread thread_t;

typedef struct proc {
    intmax_t     proc_id;
    uintptr_t    pml4;
    rg_t 		 proc_random;

    char**       environ;
    char**       argv;
    unsigned int argc;

    array_t*     fds;
    array_t*     threads;

    mmap_area_t* mem_maps;
} proc_t;

struct thread {
    tid_t tId;
    proc_t*  parent_process;
    thread_t* next_thread;
    thread_t* prev_thread;
    uint16_t tickets;

    list_t* borrowed_tickets;
    list_t* lended_tickets;

    /* Userspace information */
    ruint_t last_rip, last_rsp, last_rax, last_rdi, last_rsi, last_rdx, last_rcx;
    ruint_t last_r8, last_r9, last_r10, last_r11;
    ruint_t last_rbx, last_rbp, last_r12, last_r13, last_r14, last_r15;
    uintptr_t stack_top_address;
    uintptr_t stack_bottom_address;
};

typedef struct borrowed_ticket {
	thread_t* source;
	thread_t* target;
	uint16_t  tamount;
	bool release_now;
} borrowed_ticket_t;

#define PER_PROCESS_TICKETS 0x1000
#define BASE_STACK_SIZE 0x1000000

extern array_t* processes;

proc_t* create_init_process_structure(uintptr_t pml);
mmap_area_t* request_va_hole(proc_t* proc, uintptr_t start_address, size_t req_size);
mmap_area_t* find_va_hole(proc_t* proc, size_t req_size, size_t align_amount);
borrowed_ticket_t* transfer_tickets(thread_t* from, thread_t* to, uint16_t tamount);
int fork_process(registers_t* r, proc_t* p, thread_t* t);

void initialize_processes();
