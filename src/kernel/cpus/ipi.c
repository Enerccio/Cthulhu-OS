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
 * ipi.c
 *  Created on: Dec 26, 2015
 *      Author: Peter Vanusanik
 *  Contents: 
 */

#include "ipi.h"

#include "../processes/scheduler.h"
#include "../interrupts/idt.h"
#include "../interrupts/interrupts.h"
#include "cpu_mgmt.h"

extern void proc_spinlock_lock(volatile void* memaddr);
extern void proc_spinlock_unlock(volatile void* memaddr);
extern void kp_halt();
extern void invalidate_address(uintptr_t address);
extern uintptr_t get_active_page();
extern void set_active_page(uintptr_t address);

void ipi_received(ruint_t ecode, registers_t* registers) {
    // WATCH OUT: registers might be null if it is local interrupt
    cpu_t* cpu = get_current_cput();
    switch (cpu->apic_message_type) {
    case IPI_HALT_IMMEDIATELLY:
        cpu->apic_message_handled = 0;
        kp_halt();
    case IPI_WAKE_UP_FROM_WUA:
        registers->rax = cpu->apic_message; // unlocking from wait_until_activation if message was nonzero
        break;
    case IPI_INVALIDATE_PAGE: {
            uintptr_t active_page = get_active_page();
            uintptr_t target_page = __atomic_load_n(&cpu->apic_message_cpu->current_address_space,
                    __ATOMIC_SEQ_CST);
            if (active_page == target_page) {
                for (uintptr_t i=cpu->apic_message; i<cpu->apic_message2; i+=0x1000)
                    invalidate_address(i);
            }
        }
        break;
    case IPI_INVLD_PML: {
        uintptr_t active_page = get_active_page();
        if (active_page == cpu->apic_message) {
            set_active_page(active_page);
        }
        break;
    }
    case IPI_RUN_SCHEDULER:
        schedule(registers);
        break;
    }

    //printf("IPI received: this cpu %u, message %x, m1 %lx, m2 %lx\n", cpu->apic_id, cpu->apic_message_type,
    //      cpu->apic_message, cpu->apic_message2);

    proc_spinlock_lock(&cpu->__message_clear_lock);
    cpu->apic_message_handled = 0;
    proc_spinlock_unlock(&cpu->__message_clear_lock);
}

void send_ipi_message(uint8_t cpu_apic_id, uint8_t message_type, ruint_t message, ruint_t message2,
        registers_t* internalcall) {
    cpu_t* cpu = array_find_by_pred(cpus, search_for_cpu_by_apic, (void*)(uintptr_t)cpu_apic_id);

    if (cpu == NULL)
        return;

    if (cpu_apic_id == get_local_apic_id()) {
        proc_spinlock_lock(&cpu->__ipi_lock);
        cpu->apic_message_handled = 1;
        cpu->apic_message_type = message_type;
        cpu->apic_message = message;
        cpu->apic_message2 = message2;
        cpu->apic_message_cpu = cpu;
        ipi_received(0, internalcall);
        proc_spinlock_unlock(&cpu->__ipi_lock);
        return;
    }

    if (!cpu->started)
        return; // stopped cpu requires no interrupts

    proc_spinlock_lock(&cpu->__ipi_lock);

    proc_spinlock_lock(&cpu->__message_clear_lock);
    cpu->apic_message_handled = 1;
    proc_spinlock_unlock(&cpu->__message_clear_lock);

    cpu->apic_message_type = message_type;
    cpu->apic_message = message;
    cpu->apic_message2 = message2;
    cpu->apic_message_cpu = get_current_cput();

    send_ipi_to(cpu->apic_id, 0xFF, 0, false);

    bool clear;
    do {
        proc_spinlock_lock(&cpu->__message_clear_lock);
        clear = cpu->apic_message_handled;
        proc_spinlock_unlock(&cpu->__message_clear_lock);
    } while (clear == 1);

    proc_spinlock_unlock(&cpu->__ipi_lock);
}

void send_ipi_nowait(uint8_t cpu_apic_id, uint8_t message_type, ruint_t message, ruint_t message2,
        registers_t* internalcall) {
    cpu_t* cpu = array_find_by_pred(cpus, search_for_cpu_by_apic, (void*)(uintptr_t)cpu_apic_id);

    if (cpu == NULL)
        return;

    if (cpu_apic_id == get_local_apic_id()) {
        proc_spinlock_lock(&cpu->__ipi_lock);
        cpu->apic_message_handled = 1;
        cpu->apic_message_type = message_type;
        cpu->apic_message = message;
        cpu->apic_message2 = message2;
        cpu->apic_message_cpu = get_current_cput();
        ipi_received(0, internalcall);
        proc_spinlock_unlock(&cpu->__ipi_lock);
        return;
    }

    if (!cpu->started)
        return; // stopped cpu requires no interrupts

    proc_spinlock_lock(&cpu->__ipi_lock);

    cpu->apic_message_type = message_type;
    cpu->apic_message = message;
    cpu->apic_message2 = message2;

    send_ipi_to(cpu->apic_id, 0xFF, 0, false);

    proc_spinlock_unlock(&cpu->__ipi_lock);
}

void broadcast_ipi_message(bool self, uint8_t message_type, ruint_t message, ruint_t message2,
        registers_t* internalcall) {
    uint8_t self_apic = get_local_apic_id();
    for (unsigned int i=0; i<array_get_size(cpus); i++) {
        cpu_t* cpu = array_get_at(cpus, i);
        if (cpu->apic_id != self_apic) {
            send_ipi_message(cpu->apic_id, message_type, message, message2, NULL);
        }
    }
    if (self)
        send_ipi_message(self_apic, message_type, message, message2, internalcall);
}

void initialize_ipi_subsystem() {
    register_interrupt_handler(EXC_IPI, ipi_received);
}
