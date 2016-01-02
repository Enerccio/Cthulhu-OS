/*
 * The MIT License (MIT)
 * Copyright (c) 2015 Peter Vanusanik <admin@en-circle.com>
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy of
 * this software and associated documentation files (the "Software"), to deal in
 * the Software without restriction, including without limitation the rights to use,
 * copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the
 * Software, and to permit persons to whom the Software is furnished to do so, subject
 * to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED,
 * INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR
 * PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE
 * LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
 * TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE
 * OR OTHER DEALINGS IN THE SOFTWARE.
 *
 * idt.c
 *  Created on: Dec 23, 2015
 *      Author: Peter Vanusanik
 *  Contents: 
 */

#include "../interrupts/idt.h"

#include "../utils/rsod.h"
#include <string.h>

#include "../cpus/cpu_mgmt.h"

/** IDT gates stored in this table */
idt_gate_t idt_entries[256] __attribute__((aligned(16)));
/** ISR function vectors stored in this table */
isr_t interrupt_handlers[256] __attribute__((aligned(16)));
/** IDT PTR stored here */
idt_ptr_t idt_ptr __attribute__((aligned(16)));

extern void idt_flush(idt_ptr_t* ptr);

/**
 * Sets the IDT gate to function vector.
 *
 * gn supplies numbered interrupt, while funcall is
 * function to be called when that interrupt happens.
 */
void idt_set_gate(uint8_t gn, uintptr_t funcall) {
    idt_gate_t* gate = &idt_entries[gn];
    gate->offset015 = funcall & 0xFFFF;
    gate->offset1631 = (funcall>>16) & 0xFFFF;
    gate->offset3263 = (funcall>>32) & 0xFFFFFFFF;
    gate->selector = 8; // CODE descriptor, see GDT64.Code
    gate->flags.p = 1;
    gate->flags.ist = 1;
    gate->flags.type = 14 & 0b1111;
    gate->flags.dpl = 0;

    if (gn == 14) // page fault
    	gate->flags.ist = 1;
    if (gn == 8) // double fault
        gate->flags.ist = 2;
    if (gn == 255) // ipi fault
    	gate->flags.ist = 3;
}

/**
 * Initializes base interrupt vectors.
 *
 * Creates IDT pointer and then fills it up with generated
 * asm functions. These functions, by default, call ISR vector,
 * or kernel panics if ISR vector is unavailable.
 *
 * Also remaps IRQ to use interrupts.
 */
void initialize_interrupts() {

    memset(idt_entries, 0, sizeof(idt_entries));
    memset(interrupt_handlers, 0, sizeof(interrupt_handlers));

    idt_ptr.limit = (sizeof(idt_entries)) - 1;
    idt_ptr.base = (uintptr_t) &idt_entries;

    // GENERAL CPU INTERRUPTS
    idt_set_gate(0, (uintptr_t) isr0);
    idt_set_gate(1, (uintptr_t) isr1);
    idt_set_gate(2, (uintptr_t) isr2);
    idt_set_gate(3, (uintptr_t) isr3);
    idt_set_gate(4, (uintptr_t) isr4);
    idt_set_gate(5, (uintptr_t) isr5);
    idt_set_gate(6, (uintptr_t) isr6);
    idt_set_gate(7, (uintptr_t) isr7);
    idt_set_gate(8, (uintptr_t) isr8);
    idt_set_gate(9, (uintptr_t) isr9);

    idt_set_gate(10, (uintptr_t) isr10);
    idt_set_gate(11, (uintptr_t) isr11);
    idt_set_gate(12, (uintptr_t) isr12);
    idt_set_gate(13, (uintptr_t) isr13);
    idt_set_gate(14, (uintptr_t) isr14);
    idt_set_gate(15, (uintptr_t) isr15);
    idt_set_gate(16, (uintptr_t) isr16);
    idt_set_gate(17, (uintptr_t) isr17);
    idt_set_gate(18, (uintptr_t) isr18);
    idt_set_gate(19, (uintptr_t) isr19);

    idt_set_gate(20, (uintptr_t) isr20);
    idt_set_gate(21, (uintptr_t) isr21);
    idt_set_gate(22, (uintptr_t) isr22);
    idt_set_gate(23, (uintptr_t) isr23);
    idt_set_gate(24, (uintptr_t) isr24);
    idt_set_gate(25, (uintptr_t) isr25);
    idt_set_gate(26, (uintptr_t) isr26);
    idt_set_gate(27, (uintptr_t) isr27);
    idt_set_gate(28, (uintptr_t) isr28);
    idt_set_gate(29, (uintptr_t) isr29);

    idt_set_gate(30, (uintptr_t) isr30);
    idt_set_gate(31, (uintptr_t) isr31);
    idt_set_gate(128, (uintptr_t) isr128);
    idt_set_gate(128, (uintptr_t) isr129);
    idt_set_gate(255, (uintptr_t) isr255);

    // INTERRUPTS FROM THE BOARD
    // remapping interrupts from irq
    outb(0x20, 0x11);
    outb(0xA0, 0x11);
    outb(0x21, 0x20);
    outb(0xA1, 0x28);
    outb(0x21, 0x04);
    outb(0xA1, 0x02);
    outb(0x21, 0x01);
    outb(0xA1, 0x01);
    outb(0x21, 0x0);
    outb(0xA1, 0x0);

    idt_set_gate(32, (uintptr_t) isr32);
    idt_set_gate(33, (uintptr_t) isr33);
    idt_set_gate(34, (uintptr_t) isr34);
    idt_set_gate(35, (uintptr_t) isr35);
    idt_set_gate(36, (uintptr_t) isr36);
    idt_set_gate(37, (uintptr_t) isr37);
    idt_set_gate(38, (uintptr_t) isr38);
    idt_set_gate(39, (uintptr_t) isr39);
    idt_set_gate(40, (uintptr_t) isr40);
    idt_set_gate(41, (uintptr_t) isr41);
    idt_set_gate(42, (uintptr_t) isr42);
    idt_set_gate(43, (uintptr_t) isr43);
    idt_set_gate(44, (uintptr_t) isr44);
    idt_set_gate(45, (uintptr_t) isr45);
    idt_set_gate(46, (uintptr_t) isr46);
    idt_set_gate(47, (uintptr_t) isr47);

    idt_flush(&idt_ptr);
}

/**
 * Clears interrupt state after interrupt was handled.
 */
void pic_sendeoi(int irq) {
    switch (irq) {
    case PIC_EOI_MASTER: {
        outb(PIC1_COMMAND, 0x20);
        break;
    }
    case PIC_EOI_SLAVE: {
        outb(PIC2_COMMAND, 0x20);
        break;
    }
    case PIC_EOI_ALL: {
        pic_sendeoi(PIC_EOI_MASTER);
        pic_sendeoi(PIC_EOI_SLAVE);
        break;
    }
    default: {
        if (irq >= 8)
            outb(PIC2_COMMAND, 0x20);
        outb(PIC1_COMMAND, 0x20);
        break;
    }
    }
}

/**
 * ISR common handler.
 *
 * Called by interrupt vector. Registers are editable state of
 * CPU before interrupt.
 */
void isr_handler(registers_t* r) {
    if (interrupt_handlers[r->type] == 0) {
        error(ERROR_NO_IV_FOR_INTERRUPT, r->type, r->ecode, &r);
    } else {
        interrupt_handlers[r->type](r->ecode, r);
    }

    if (r->type > 31 && r->type < 48) {
		if (r->type >= 40 && r->type != 47)
			pic_sendeoi(PIC_EOI_SLAVE);
		if (r->type != 39)
			pic_sendeoi(PIC_EOI_MASTER);
	} else if (r->type == 255) {
		// ipi interrupt
		volatile uint32_t* eoi = (uint32_t*)physical_to_virtual(apicaddr+0xB0);
		*eoi = 0;
	}
}

/**
 * Registers ISR vector for interrupt.
 *
 * Will overwrite interrupt handler set up before.
 */
void register_interrupt_handler(uint8_t interrupt_id, isr_t handler_func) {
    interrupt_handlers[interrupt_id] = handler_func;
}
