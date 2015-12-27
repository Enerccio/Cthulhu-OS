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

#include "../interrupts/idt.h"
#include "../interrupts/interrupts.h"
#include "cpu_mgmt.h"

extern void proc_spinlock_lock(volatile void* memaddr);
extern void proc_spinlock_unlock(volatile void* memaddr);
extern void kp_halt();

void ipi_received(uint64_t ecode, registers_t* registers) {
	// WATCH OUT: registers might be null if it is local interrupt
	cpu_t* cpu = get_current_cput();
	switch (cpu->apic_message_type) {
	case IPI_HALT_IMMEDIATELLY:
		cpu->apic_message_handled = 0;
		kp_halt();
	case IPI_WAKE_UP_FROM_WUA:
		registers->rax = cpu->apic_message; // unlocking from wait_until_activation if message was nonzero
		break;
	}

	cpu->apic_message_handled = 0;
}

void send_ipi_message(uint8_t cpu_apic_id, uint8_t message_type, uint64_t message) {
	cpu_t* cpu = array_find_by_pred(cpus, search_for_cpu_by_apic, (void*)(uintptr_t)cpu_apic_id);

	if (cpu == NULL)
		return;

	if (cpu_apic_id == get_local_apic_id()) {
		proc_spinlock_lock(&cpu->__cpu_lock);
		cpu->apic_message_handled = 1;
		cpu->apic_message_type = message_type;
		cpu->apic_message = message;
		ipi_received(0, NULL);
		proc_spinlock_unlock(&cpu->__cpu_lock);
	}

	if (!cpu->started)
		return; // stopped cpu requires no interrupts

	proc_spinlock_lock(&cpu->__cpu_lock);

	cpu->apic_message_handled = 1;
	cpu->apic_message_type = message_type;
	cpu->apic_message = message;
	send_ipi_to(cpu->apic_id, 0xFF, 0, false);

	while (cpu->apic_message_handled == 1)
		;

	proc_spinlock_unlock(&cpu->__cpu_lock);
}

void initialize_ipi_subsystem() {
	register_interrupt_handler(EXC_IPI, ipi_received);
}
