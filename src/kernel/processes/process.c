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
#include "../utils/rsod.h"
#include "../memory/paging.h"
#include "scheduler.h"
#include "../loader/elf.h"
#include "../syscalls/sys.h"

#include <stdatomic.h>
#include <errno.h>

ruint_t __proclist_lock;
ruint_t process_id_num;
ruint_t thread_id_num;
list_t* processes;

extern void proc_spinlock_lock(volatile void* memaddr);
extern void proc_spinlock_unlock(volatile void* memaddr);
extern void set_active_page(void* address);
extern void* get_active_page();
extern ruint_t __thread_modifier;

pid_t get_current_pid() {
    cpu_t* cpu = get_current_cput();
    proc_spinlock_lock(&cpu->__cpu_lock);
    proc_spinlock_lock(&cpu->__cpu_sched_lock);
    proc_spinlock_lock(&__thread_modifier);

    pid_t pid = cpu->ct == NULL ? -1 : cpu->ct->parent_process->proc_id;

    proc_spinlock_unlock(&__thread_modifier);
    proc_spinlock_unlock(&cpu->__cpu_lock);
    proc_spinlock_unlock(&cpu->__cpu_sched_lock);
    return pid;
}

proc_t* create_init_process_structure(uintptr_t pml) {
    proc_t* process = malloc(sizeof(proc_t));
    if (process == NULL) {
        error(ERROR_MINIMAL_MEMORY_FAILURE, 0, 0, &create_init_process_structure);
    }

    process->continuation = malloc(sizeof(continuation_t));
    if (process->continuation == NULL) {
    	error(ERROR_MINIMAL_MEMORY_FAILURE, 0, 0, &create_init_process_structure);
    }
    process->continuation->present = false;

    proc_spinlock_lock(&__proclist_lock);
    process->proc_id = ++process_id_num;
    list_push_right(processes, process);
    proc_spinlock_unlock(&__proclist_lock);

    process->fds = create_array();
    if (process->fds == NULL) {
        error(ERROR_MINIMAL_MEMORY_FAILURE, 0, 0, &create_init_process_structure);
    }
    process->threads = create_array();
    if (process->threads == NULL) {
        error(ERROR_MINIMAL_MEMORY_FAILURE, 0, 0, &create_init_process_structure);
    }
    process->pml4 = pml;
    process->mem_maps = NULL;
    process->proc_random = rg_create_random_generator(get_unix_time());
    process->parent = NULL;
    process->priority = 0;
    process->mutexes = create_uint64_table();
    if (process->mutexes == NULL) {
    	error(ERROR_MINIMAL_MEMORY_FAILURE, 0, 0, &create_init_process_structure);
    }

    thread_t* main_thread = malloc(sizeof(thread_t));
    if (main_thread == NULL) {
        error(ERROR_MINIMAL_MEMORY_FAILURE, 0, 0, &create_init_process_structure);
    }
    memset(main_thread, 0, sizeof(thread_t));
    main_thread->parent_process = process;
    main_thread->tId = __atomic_add_fetch(&thread_id_num, 1, __ATOMIC_SEQ_CST);
    main_thread->priority = 0;
    main_thread->blocked = false;
    main_thread->last_rdi = (ruint_t)(uintptr_t)process->argc;
    main_thread->last_rsi = (ruint_t)(uintptr_t)process->argv;
    main_thread->last_rdx = (ruint_t)(uintptr_t)process->environ;
    if (array_push_data(process->threads, main_thread) == 0) {
    	error(ERROR_MINIMAL_MEMORY_FAILURE, 0, 0, &create_init_process_structure);
    }

    return process;
}

static struct chained_element* __process_get_function(void* data){
	return &((proc_t*)data)->process_list;
}

void initialize_processes() {
    __proclist_lock = 0;
    process_id_num = 0;
    thread_id_num = 0;
    processes = create_list_static(&__process_get_function);
}

mmap_area_t** mmap_area(proc_t* proc, uintptr_t address) {
	mmap_area_t** lm = &proc->mem_maps;
	uintptr_t x1, x2, y1, y2;

	while (*lm != NULL) {
		mmap_area_t* mmap = *lm;

		x1 = address;
		x2 = address;
		y1 = mmap->vastart;
		y2 = mmap->vaend;

		if (address >= mmap->vaend) {
			lm = &mmap->next;
			continue;
		} else if (x1 <= y2 && y1 <= x2) {
			// section overlaps
			return lm;
		} else {
			break;
		}
	}

	return NULL;
}

mmap_area_t** request_va_hole(proc_t* proc, uintptr_t start_address, size_t req_size) {
    mmap_area_t** lm = &proc->mem_maps;
    uintptr_t x1, x2, y1, y2;

    if (start_address < 0x1000)
    	return NULL;

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
    newmm->count = 1;
    *lm = newmm;
    return lm;
}

#define ALIGN_DOWN(a, b) ((a) - (a % b))
#define ALIGN_UP(a, b) ((a % b == 0) ? (a) : (ALIGN_DOWN(a, b) + b))

mmap_area_t** find_va_hole(proc_t* proc, size_t req_size, size_t align_amount) {
    mmap_area_t** lm = &proc->mem_maps;
    uintptr_t start_address = 0x1000;
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
    newmm->count = 1;
    *lm = newmm;
    return lm;
}

void* proc_alloc(size_t size) {
    cpu_t* cpu = get_current_cput();
    proc_spinlock_lock(&cpu->__cpu_lock);
    proc_spinlock_lock(&cpu->__cpu_sched_lock);
    proc_spinlock_lock(&__thread_modifier);

    proc_t* proc = cpu->ct->parent_process;
    void* addr = proc_alloc_direct(proc, size);

    proc_spinlock_unlock(&__thread_modifier);
    proc_spinlock_unlock(&cpu->__cpu_lock);
    proc_spinlock_unlock(&cpu->__cpu_sched_lock);
    return addr;
}

void* proc_alloc_direct(proc_t* proc, size_t size) {
    mmap_area_t** _hole = find_va_hole(proc, size, 16);
    mmap_area_t* hole = *_hole;
    hole->mtype = kernel_allocated_heap_data;
    allocate(hole->vastart, size, false, false);
    return (void*)hole->vastart;
}


mmap_area_t* free_mmap_area(mmap_area_t* mm, mmap_area_t** pmma) {
	uint64_t use_count = __atomic_sub_fetch(&mm->count, 1, __ATOMIC_SEQ_CST);
	switch (mm->mtype) {
	case program_data:
	case stack_data:
	case heap_data:
	case kernel_allocated_heap_data: {
		deallocate(mm->vastart, mm->vaend-mm->vastart);
	} break;
	case nondealloc_map:
		break;
	}
	mmap_area_t* mmn = mm->next;
	*pmma = mm->next;
	if (use_count == 0) {
		free(mm);
	}
	return mmn;
}

void proc_dealloc(uintptr_t mem) {
	cpu_t* cpu = get_current_cput();
	proc_spinlock_lock(&cpu->__cpu_lock);
	proc_spinlock_lock(&cpu->__cpu_sched_lock);
	proc_spinlock_lock(&__thread_modifier);

	proc_t* proc = cpu->ct->parent_process;
	proc_dealloc_direct(proc, mem);

	proc_spinlock_unlock(&__thread_modifier);
	proc_spinlock_unlock(&cpu->__cpu_lock);
	proc_spinlock_unlock(&cpu->__cpu_sched_lock);
}

void proc_dealloc_direct(proc_t* proc, uintptr_t mem) {
	mmap_area_t** _hole = mmap_area(proc, mem);
	mmap_area_t* hole = *_hole;
	if (hole != NULL) {
		free_mmap_area(hole, _hole);
	}
}

static int cpy_array(int count, char*** a) {
    char** array = *a;
    char** na = malloc(8*(count+1));
    if (na == NULL)
        return ENOMEM_INTERNAL;

    int i = 0;
    for (; i<count; i++) {
        char* cpy = malloc(strlen(array[i])+1);
        if (cpy == NULL) {
            for (int j=0; j<i; j++) {
                free(na[j]);
            }
            free(na);
            return ENOMEM_INTERNAL;
        }
        memcpy(cpy, array[i], strlen(array[i])+1);
        na[i] = cpy;
    }
    na[count] = NULL;
    *a = na;
    return 0;
}

static int cpy_array_user(int count, char*** a, proc_t* p) {
    char** array = *a;
    char** na = proc_alloc_direct(p, 8*(count+1));
    if (na == NULL)
        return ENOMEM_INTERNAL;

    int i = 0;
    for (; i<count; i++) {
        char* cpy = proc_alloc_direct(p, strlen(array[i])+1);
        if (cpy == NULL) {
            return ENOMEM_INTERNAL;
        }
        memcpy(cpy, array[i], strlen(array[i])+1);
        na[i] = cpy;
    }
    na[count] = NULL;
    *a = na;
    return 0;
}

void free_proc_memory(proc_t* proc) {
    mmap_area_t* mm = proc->mem_maps;
    mmap_area_t** pmm = &proc->mem_maps;
    while (mm != NULL) {
    	mm = free_mmap_area(mm, pmm);
    }
}

static void free_array(int count, char** a) {
    for (int i=0; i<count; i++) {
        free(a[i]);
    }
    free(a);
}

int create_process_base(uint8_t* image_data, int argc, char** argv,
		char** envp, proc_t** cpt, uint8_t asked_priority, registers_t* r) {
	proc_spinlock_lock(&__thread_modifier);
	uint8_t cpp = get_current_cput()->ct->parent_process->priority;
	proc_spinlock_unlock(&__thread_modifier);

	if (cpp > asked_priority) {
		return EINVAL;
	}

	int envc = 0;
	char** envt = envp;
	while (*envt != NULL) {
		++envc;
		++envt;
	}

	int err;
	if ((err = cpy_array(argc, &argv)) != 0)
		return err;
	if ((err = cpy_array(envc, &envp)) != 0) {
		free_array(argc, argv);
		return err;
	}
	// envp and argv are now kernel structures

	proc_t* process = malloc(sizeof(proc_t));
	if (process == NULL) {
		return ENOMEM_INTERNAL;
	}

	process->fds = create_array();
	if (process->fds == NULL) {
		free(process);
		return ENOMEM_INTERNAL;
	}

	process->threads = create_array();
	if (process->fds == NULL) {
		destroy_array(process->fds);
		free(process);
		return ENOMEM_INTERNAL;
	}

	process->mem_maps = NULL;
	process->proc_random = rg_create_random_generator(get_unix_time());
	process->parent = NULL;
	process->priority = asked_priority;
	process->pml4 = create_pml4();
	if (process->pml4 == 0) {
		destroy_array(process->fds);
		free(process);
		return ENOMEM_INTERNAL;
	}

	uintptr_t opml4 = (uintptr_t)get_active_page();
	process->mutexes = create_uint64_table();

	if (process->mutexes == NULL) {
		destroy_array(process->threads);
		destroy_array(process->fds);
		free(process);
		// TODO: free process address page
		set_active_page((void*)opml4);
		return ENOMEM_INTERNAL;
	}

	set_active_page((void*)process->pml4);

	thread_t* main_thread = malloc(sizeof(thread_t));
	if (main_thread == NULL) {
		destroy_table(process->mutexes);
		destroy_array(process->threads);
		destroy_array(process->fds);
		free(process);
		// TODO: free process address page
		set_active_page((void*)opml4);
		return ENOMEM_INTERNAL;
	}
	memset(main_thread, 0, sizeof(thread_t));
	main_thread->parent_process = process;
	main_thread->priority = asked_priority;
	main_thread->blocked = false;
	array_push_data(process->threads, main_thread);

	err = load_elf_exec((uintptr_t)image_data, process);
	if (err == ELF_ERROR_ENOMEM) {
		err = ENOMEM_INTERNAL;
	} else if (err != 0) {
		err = EINVAL;
	}

	if (err != 0) {
		free(main_thread);
		destroy_table(process->mutexes);
		destroy_array(process->threads);
		destroy_array(process->fds);
		free(process);
		// TODO: free process address page
		set_active_page((void*)opml4);
		return err;
	}

	char** argvu = argv;
	char** envpu = envp;
	if ((err = cpy_array_user(argc, &argvu, process)) != 0) {
		free(main_thread);
		destroy_table(process->mutexes);
		destroy_array(process->threads);
		destroy_array(process->fds);
		free(process);
		// TODO: free process address page
		set_active_page((void*)opml4);
		return err;
	}
	if ((err = cpy_array_user(envc, &envpu, process)) != 0) {
		free(main_thread);
		destroy_table(process->mutexes);
		destroy_array(process->threads);
		destroy_array(process->fds);
		free(process);
		// TODO: free process address page
		set_active_page((void*)opml4);
		return err;
	}

	process->argc = argc;
	process->argv = argvu;
	process->environ = envpu;

	main_thread->last_r12 = 0;
	main_thread->last_r11 = 0;
	main_thread->last_r10 = 0;
	main_thread->last_r9 = 0;
	main_thread->last_r8 = 0;
	main_thread->last_rax = 0;
	main_thread->last_rbx = 0;
	main_thread->last_rcx = 0;
	main_thread->last_rdx = 0;
	main_thread->last_rdi = 0;
	main_thread->last_rsi = 0;
	main_thread->last_rbp = 0;
	main_thread->last_rflags = 0x200; // enable interrupts

	proc_spinlock_lock(&__proclist_lock);
	main_thread->tId = __atomic_add_fetch(&thread_id_num, 1, __ATOMIC_SEQ_CST);
	process->proc_id = ++process_id_num;
	list_push_right(processes, process);
	proc_spinlock_unlock(&__proclist_lock);

	main_thread->last_rdi = (ruint_t)(uintptr_t)process->argc;
	main_thread->last_rsi = (ruint_t)(uintptr_t)process->argv;
	main_thread->last_rdx = (ruint_t)(uintptr_t)process->environ;

	free_array(argc, argv);
	free_array(envc, envp);

	// TODO: add split option?
	main_thread->last_rax = process->proc_id;
	enschedule_best(main_thread);
	*cpt = process;
	set_active_page((void*)opml4);
	return 0;
}

uintptr_t map_virtual_virtual(uintptr_t* _vastart, uintptr_t vaend, bool readonly) {
	uintptr_t vastart = *_vastart;
    uintptr_t vaoffset = vastart % 0x1000;
    vastart = ALIGN_DOWN(vastart, 0x1000);
    vaend = ALIGN_UP(vaend, 0x1000);

    cpu_t* cpu = get_current_cput();
    proc_spinlock_lock(&cpu->__cpu_lock);
    proc_spinlock_lock(&cpu->__cpu_sched_lock);
    proc_spinlock_lock(&__thread_modifier);

    proc_t* proc = cpu->ct->parent_process;

    proc_spinlock_unlock(&__thread_modifier);
    proc_spinlock_unlock(&cpu->__cpu_lock);
    proc_spinlock_unlock(&cpu->__cpu_sched_lock);

    mmap_area_t** _hole = find_va_hole(proc, vaend-vastart, 0x1000);
    mmap_area_t* hole = *_hole;
    if (hole == NULL) {
		return 0;
	}
    hole->mtype = kernel_allocated_heap_data;
    uintptr_t temporary = hole->vastart;
    if (!map_range(_vastart, vaend, &temporary, hole->vaend, true, readonly, false)) {
    	free_mmap_area(hole, _hole);
    	return 0;
    }
    return hole->vastart+vaoffset;
}

uintptr_t map_physical_virtual(puint_t* _vastart, puint_t vaend, bool readonly) {
	puint_t vastart = *_vastart;
    uintptr_t vaoffset = vastart % 0x1000;
    vastart = ALIGN_DOWN(vastart, 0x1000);
    vaend = ALIGN_UP(vaend, 0x1000);

    cpu_t* cpu = get_current_cput();
    proc_spinlock_lock(&cpu->__cpu_lock);
    proc_spinlock_lock(&cpu->__cpu_sched_lock);
    proc_spinlock_lock(&__thread_modifier);

    proc_t* proc = cpu->ct->parent_process;

    proc_spinlock_unlock(&__thread_modifier);
    proc_spinlock_unlock(&cpu->__cpu_lock);
    proc_spinlock_unlock(&cpu->__cpu_sched_lock);

    mmap_area_t** _hole = find_va_hole(proc, vaend-vastart, 0x1000);
    mmap_area_t* hole = *_hole;
    if (hole == NULL) {
    	return 0;
    }
    hole->mtype = kernel_allocated_heap_data;
    uintptr_t temporary = hole->vastart;
    if (!map_range(_vastart, vaend, &temporary, hole->vaend, false, readonly, false)) {
    	free_mmap_area(hole, _hole);
    	return 0;
    }
    return hole->vastart+vaoffset;
}
