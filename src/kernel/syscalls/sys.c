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
 * sys.c
 *  Created on: Dec 27, 2015
 *      Author: Peter Vanusanik
 *  Contents: 
 */

#include "sys.h"
#include "../memory/paging.h"
#include "../processes/process.h"
#include "../cpus/cpu_mgmt.h"
#include "../interrupts/idt.h"

extern ruint_t __thread_modifier;
extern void proc_spinlock_lock(volatile void* memaddr);
extern void proc_spinlock_unlock(volatile void* memaddr);

#include "syscall_defs.cc"

syscall_t syscalls [512];

void register_syscall(bool system, uint8_t syscall_id, syscall_t syscall) {
    if (syscall.uses_error && syscall.args == 0) {
        // this has no sense
        return;
    }
    uint16_t sysid = system ? (256 + syscall_id) : syscall_id;
    syscall.present = true;
    syscalls[sysid] = syscall;
}

void sys_handler(registers_t* registers, bool dev) {
    if (registers->rax >= 256) {
        registers->rdi = -1;
        return;
    }
    uint16_t rnum = dev ? (256 + registers->rax) : registers->rax;
    if (!syscalls[rnum].present) {
        registers->rdi = -1;
        return;
    }

    syscall_t* sc = &syscalls[rnum];
    if (sc->uses_error) {
        int error;
        switch (sc->args) {
        case 1: registers->rax = sc->syscall._1(registers, (ruint_t)&error);
            break;
        case 2: registers->rax = sc->syscall._2(registers, (ruint_t)&error, registers->rdi);
            break;
        case 3: registers->rax = sc->syscall._3(registers, (ruint_t)&error, registers->rdi, registers->rsi);
                break;
        case 4: registers->rax = sc->syscall._4(registers, (ruint_t)&error, registers->rdi, registers->rsi, registers->rdx);
                break;
        case 5: registers->rax = sc->syscall._5(registers, (ruint_t)&error, registers->rdi,
                                                registers->rsi, registers->rdx, registers->rcx);
                break;
        }
    } else {
        switch (sc->args) {
        case 0: registers->rax = sc->syscall._0(registers);
            break;
        case 1: registers->rax = sc->syscall._1(registers, registers->rdi);
            break;
        case 2: registers->rax = sc->syscall._2(registers, registers->rdi, registers->rsi);
            break;
        case 3: registers->rax = sc->syscall._3(registers, registers->rdi, registers->rsi, registers->rdx);
                break;
        case 4: registers->rax = sc->syscall._4(registers, registers->rdi, registers->rsi, registers->rdx, registers->rcx);
                break;
        case 5: registers->rax = sc->syscall._5(registers, registers->rdi,
                                                registers->rsi, registers->rdx, registers->rcx, registers->r9);
                break;
        }
    }
}

void system_call_handler(uintptr_t error_code, registers_t* r) {
    sys_handler(r, false);
}

void dev_system_call_handler(uintptr_t error_code, registers_t* r) {
    sys_handler(r, true);
}

syscall_t make_syscall_0(syscall_0 sfnc, bool e, bool sched_after) {
    syscall_t syscall;
    syscall.args = 0;
    syscall.schedule_after = sched_after;
    syscall.uses_error = e;
    syscall.syscall._0 = sfnc;
    return syscall;
}

syscall_t make_syscall_1(syscall_1 sfnc, bool e, bool sched_after) {
    syscall_t syscall;
    syscall.args = 1;
    syscall.schedule_after = sched_after;
    syscall.uses_error = e;
    syscall.syscall._1 = sfnc;
    return syscall;
}

syscall_t make_syscall_2(syscall_2 sfnc, bool e, bool sched_after) {
    syscall_t syscall;
    syscall.args = 2;
    syscall.schedule_after = sched_after;
    syscall.uses_error = e;
    syscall.syscall._2 = sfnc;
    return syscall;
}

syscall_t make_syscall_3(syscall_3 sfnc, bool e, bool sched_after) {
    syscall_t syscall;
    syscall.args = 3;
    syscall.schedule_after = sched_after;
    syscall.uses_error = e;
    syscall.syscall._3 = sfnc;
    return syscall;
}

syscall_t make_syscall_4(syscall_4 sfnc, bool e, bool sched_after) {
    syscall_t syscall;
    syscall.args = 4;
    syscall.schedule_after = sched_after;
    syscall.uses_error = e;
    syscall.syscall._4 = sfnc;
    return syscall;
}

syscall_t make_syscall_5(syscall_5 sfnc, bool e, bool sched_after) {
    syscall_t syscall;
    syscall.args = 5;
    syscall.schedule_after = sched_after;
    syscall.syscall._5 = sfnc;
    return syscall;
}

void initialize_system_calls() {
    memset(syscalls, 0, sizeof(syscalls));
    register_interrupt_handler(0x80, system_call_handler);
    register_interrupt_handler(0x81, dev_system_call_handler);

    register_syscall(false, SYS_MEMALLOC, make_syscall_1(allocate_memory, false, false));
    register_syscall(false, SYS_MEMDEALLOC, make_syscall_2(deallocate_memory, false, false));
    register_syscall(false, SYS_GET_TID, make_syscall_0(get_tid, false, false));
    register_syscall(false, SYS_FORK, make_syscall_2(fork, true, false));
}
