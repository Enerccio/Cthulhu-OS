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
#include "../memory/paging.h"
#include "ipc.h"

#include <ds/array.h>
#include <ds/random.h>
#include <ds/llist.h>
#include <ds/hmap.h>
#include <ds/queue.h>

#include <cthulhu/messages.h>
#include <cthulhu/threading.h>

#define MESSAGE_BUFFER_CNT (10)

typedef struct file_descriptor {
    uint32_t                fd_pid;
    uint64_t                owner_daemon;
} fd_t;

typedef enum mmap_area_type {
    program_data, stack_data, heap_data, nondealloc_map, kernel_allocated_heap_data
} ma_type_t;

typedef struct mmap_area {
    uintptr_t               vastart;
    uintptr_t               vaend;
    ma_type_t               mtype;
    uint64_t                count;
    struct mmap_area*       next;
} mmap_area_t;

typedef struct thread thread_t;

typedef struct continuation continuation_t;

typedef struct proc {
    intmax_t                proc_id;
    struct proc*            parent;
    uintptr_t               pml4;
    rg_t                    proc_random;

    char**                  environ;
    char**                  argv;
    unsigned int            argc;

    array_t*                fds;
    array_t*                threads;
    uint8_t                 priority;

    mmap_area_t*            mem_maps;
    struct chained_element  process_list;

    _message_t                 output_buffer[MESSAGE_BUFFER_CNT];
    queue_t*                   input_buffer;
    queue_t*                   pq_input_buffer;
    message_target_container_t groups[256];

    hash_table_t*           futexes;
    list_t*                 blocked_wait_messages;
} proc_t;

struct thread {
    tid_t                   tId;
    proc_t*                 parent_process;

    uint8_t                 priority;
    bool                    blocked;
    continuation_t*         continuation;

    /* Userspace information */
    ruint_t                 last_rip, last_rsp, last_rax, last_rdi, last_rsi, last_rdx, last_rcx;
    ruint_t                 last_r8, last_r9, last_r10, last_r11;
    ruint_t                 last_rbx, last_rbp, last_r12, last_r13, last_r14, last_r15, last_rflags;

    uintptr_t               stack_top_address;
    uintptr_t               stack_bottom_address;

    struct chained_element  schedule_list;
    struct chained_element  blocked_list;

    tli_t*                  local_info;
    list_t*                 futex_block;
};

#define BASE_STACK_SIZE 0x1000000

extern list_t* processes;

pid_t get_current_pid();
proc_t* get_current_process();

proc_t* create_init_process_structure(uintptr_t pml);
void process_init(proc_t* process);

mmap_area_t** mmap_area(proc_t* proc, uintptr_t address);
mmap_area_t** request_va_hole(proc_t* proc, uintptr_t start_address, size_t req_size);
mmap_area_t** find_va_hole(proc_t* proc, size_t req_size, size_t align_amount);

int create_process_base(uint8_t* image_data, int argc, char** argv, char** envp, proc_t** cpt,
        uint8_t priority, registers_t* r);

void* proc_alloc(size_t size);
void* proc_alloc_direct(proc_t* proc, size_t size);
void  proc_dealloc(uintptr_t mem);
void  proc_dealloc_direct(proc_t* proc, uintptr_t mem);

uintptr_t map_virtual_virtual(uintptr_t* vastart, uintptr_t vaend, bool readonly);
uintptr_t map_physical_virtual(puint_t* vastart, puint_t vaend, bool readonly);

void initialize_processes();
