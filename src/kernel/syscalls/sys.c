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
#include "../processes/daemons.h"

extern ruint_t __thread_modifier;
extern void proc_spinlock_lock(volatile void* memaddr);
extern void proc_spinlock_unlock(volatile void* memaddr);
extern void register_syscall_handler();

syscall_t syscalls [4096];

#include "syscall_defs.cc"

void register_syscall(bool system, uint16_t syscall_id, syscall_t syscall) {
    if (syscall.uses_error && syscall.args == 0) {
        // this has no sense
        return;
    }
    uint16_t sysid = system ? syscall_id : syscall_id;
    syscall.present = true;
    syscalls[sysid] = syscall;
}

void do_sys_handler(registers_t* registers, syscall_t* sc, continuation_t* cnt) {
	if (sc->uses_error) {
		int error;
		switch (sc->args) {
		case 1: registers->rax = sc->syscall._1(registers, cnt, (ruint_t)&error);
			registers->rsi = (ruint_t)error;
			break;
		case 2: registers->rax = sc->syscall._2(registers, cnt, (ruint_t)&error, registers->rdi);
			registers->rdx = (ruint_t)error;
			break;
		case 3: registers->rax = sc->syscall._3(registers, cnt, (ruint_t)&error, registers->rdi,
												registers->rsi);
				registers->r8 = (ruint_t)error;
				break;
		case 4: registers->rax = sc->syscall._4(registers, cnt, (ruint_t)&error, registers->rdi,
												registers->rsi, registers->rdx);
				registers->r8 = (ruint_t)error;
				break;
		case 5: registers->rax = sc->syscall._5(registers, cnt, (ruint_t)&error, registers->rdi,
												registers->rsi, registers->rdx, registers->r8);
				registers->r9 = (ruint_t)error;
				break;
		}
	} else {
		switch (sc->args) {
		case 0: registers->rax = sc->syscall._0(registers, cnt);
			break;
		case 1: registers->rax = sc->syscall._1(registers, cnt, registers->rdi);
			break;
		case 2: registers->rax = sc->syscall._2(registers, cnt, registers->rdi, registers->rsi);
			break;
		case 3: registers->rax = sc->syscall._3(registers, cnt, registers->rdi, registers->rsi,
												registers->rdx);
				break;
		case 4: registers->rax = sc->syscall._4(registers, cnt, registers->rdi, registers->rsi,
												registers->rdx, registers->r8);
				break;
		case 5: registers->rax = sc->syscall._5(registers, cnt, registers->rdi,
												registers->rsi, registers->rdx, registers->r8, registers->r9);
				break;
		}
	}
}

void sys_handler(registers_t* registers) {
    if (registers->rax >= 4096) {
        registers->rdi = -1;
        return;
    }
    uint16_t rnum = registers->rax;
    if (!syscalls[rnum].present) {
        registers->rdi = -1;
        return;
    }

    if (rnum == SYS_ALLOC_CONT) {
    	registers->rax = EINVAL;
    	return;
    }

    syscall_t* sc = &syscalls[rnum];
    cpu_t* cpu = get_current_cput();

    proc_spinlock_lock(&cpu->__cpu_lock);
    proc_spinlock_lock(&cpu->__cpu_sched_lock);
	proc_spinlock_lock(&__thread_modifier);

	thread_t* ct = cpu->ct;
	continuation_t* cnt = ct->continuation;
	cnt->continuation = *sc;
	cnt->_0 = registers->rdi;
	cnt->_1 = registers->rsi;
	cnt->_2 = registers->r8;
	cnt->_3 = registers->r9;
	cnt->_4 = registers->r10;

	proc_spinlock_unlock(&__thread_modifier);
	proc_spinlock_unlock(&cpu->__cpu_sched_lock);
	proc_spinlock_unlock(&cpu->__cpu_lock);

    do_sys_handler(registers, sc, cnt);
}

syscall_t make_syscall_0(syscall_0 sfnc, bool e, bool unsafe) {
    syscall_t syscall;
    syscall.args = 0;
    syscall.uses_error = e;
    syscall.syscall._0 = sfnc;
    syscall.unsafe = unsafe;
    return syscall;
}

syscall_t make_syscall_1(syscall_1 sfnc, bool e, bool unsafe) {
    syscall_t syscall;
    syscall.args = 1;
    syscall.uses_error = e;
    syscall.syscall._1 = sfnc;
    syscall.unsafe = unsafe;
    return syscall;
}

syscall_t make_syscall_2(syscall_2 sfnc, bool e, bool unsafe) {
    syscall_t syscall;
    syscall.args = 2;
    syscall.uses_error = e;
    syscall.syscall._2 = sfnc;
    syscall.unsafe = unsafe;
    return syscall;
}

syscall_t make_syscall_3(syscall_3 sfnc, bool e, bool unsafe) {
    syscall_t syscall;
    syscall.args = 3;
    syscall.uses_error = e;
    syscall.syscall._3 = sfnc;
    syscall.unsafe = unsafe;
    return syscall;
}

syscall_t make_syscall_4(syscall_4 sfnc, bool e, bool unsafe) {
    syscall_t syscall;
    syscall.args = 4;
    syscall.uses_error = e;
    syscall.syscall._4 = sfnc;
    syscall.unsafe = unsafe;
    return syscall;
}

syscall_t make_syscall_5(syscall_5 sfnc, bool e, bool unsafe) {
    syscall_t syscall;
    syscall.args = 5;
    syscall.syscall._5 = sfnc;
    syscall.unsafe = unsafe;
    return syscall;
}

void initialize_system_calls() {
	register_syscall_handler();
    memset(syscalls, 0, sizeof(syscalls));

    register_syscall(false, SYS_ALLOCATE, make_syscall_1(allocate_memory, false, false));
    register_syscall(false, SYS_ALLOC_CONT, make_syscall_3(allocate_memory_cont, false, false));
    register_syscall(false, SYS_DEALLOCATE, make_syscall_2(deallocate_memory, false, false));
    register_syscall(false, SYS_GET_PID, make_syscall_0(get_pid, false, false));
    register_syscall(false, SYS_SEND_MESSAGE, make_syscall_1(sys_send_message, false, true));

    register_syscall(false, SYS_GET_CTHREAD_PRIORITY, make_syscall_0(get_ct_priority, false, false));
    register_syscall(false, SYS_FUTEX_WAIT, make_syscall_2(__futex_wait, false, false));
    register_syscall(false, SYS_FUTEX_WAKE, make_syscall_2(__futex_wake, false, false));

    // dev syscalls
    register_syscall(true, DEV_SYS_FRAMEBUFFER_GET_HEIGHT, make_syscall_0(dev_fb_get_height, false, false));
    register_syscall(true, DEV_SYS_FRAMEBUFFER_GET_WIDTH, make_syscall_0(dev_fb_get_width, false, false));
    register_syscall(true, DEV_SYS_IVFS_GET_PATH_ELEMENT, make_syscall_2(get_initramfs_entry, false, true));
    register_syscall(true, DEV_SYS_SERVICE_EXISTS, make_syscall_1(get_service_status, false, true));
    register_syscall(true, DEV_SYS_SERVICE_REGISTER, make_syscall_1(register_service, false, true));
    register_syscall(true, DEV_SYS_INITRAMFS_EXECVE, make_syscall_4(create_process_ivfs, false, true));
    register_syscall(true, DEV_SYS_PCIe_BUS_COUNT, make_syscall_0(dev_dm_get_pcie_c, false, false));
    register_syscall(true, DEV_SYS_PCIe_INFO, make_syscall_1(dev_dm_get_pcie_info, false, false));
    register_syscall(true, DEV_SYS_MAP_PHYSICAL_SELF, make_syscall_2(dev_selfmap_physical, false, false));
}
