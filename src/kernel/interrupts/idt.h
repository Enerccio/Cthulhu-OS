/*
 * The MIT License (MIT)
 * Copyright (c) 2015 Peter Vanusanik <admin@en-circle.com>
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation the
 * rights to use, copy, modify, merge, publish, distribute, sublicense, and/or
 * sell copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS
 * FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
 * COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
 * IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
 * WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 *
 * idt.h
 *  Created on: Dec 23, 2015
 *      Author: Peter Vanusanik
 *  Contents: interrupt description table
 */

#pragma once

#include "../commons.h"

struct idt_gate {
    uint16_t offset015;
    uint16_t selector;
    struct {
        uint16_t ist  : 3; // interrupt stack table
        uint16_t r0   : 1;
        uint16_t r1   : 1;
        uint16_t r2   : 3;
        uint16_t type : 4;
        uint16_t r3   : 1;
        uint16_t dpl  : 2; // descriptor priviledge level
        uint16_t p    : 1; // segment present flag
    } __attribute__((packed)) flags;
    uint16_t offset1631;
    uint32_t offset3263;
    uint32_t reserved;
} __attribute__((packed));
typedef struct idt_gate idt_gate_t;

struct idt_ptr {
    uint16_t limit;
    uint64_t base;
}__attribute__((packed));
typedef struct idt_ptr idt_ptr_t;

void initialize_interrupts();

extern void isr0();
extern void isr1();
extern void isr2();
extern void isr3();
extern void isr4();
extern void isr5();
extern void isr6();
extern void isr7();
extern void isr8();
extern void isr9();
extern void isr10();
extern void isr11();
extern void isr12();
extern void isr13();
extern void isr14();
extern void isr15();
extern void isr16();
extern void isr17();
extern void isr18();
extern void isr19();
extern void isr20();
extern void isr21();
extern void isr22();
extern void isr23();
extern void isr24();
extern void isr25();
extern void isr26();
extern void isr27();
extern void isr28();
extern void isr29();
extern void isr30();
extern void isr31();
extern void isr32();
extern void isr33();
extern void isr34();
extern void isr35();
extern void isr36();
extern void isr37();
extern void isr38();
extern void isr39();
extern void isr40();
extern void isr41();
extern void isr42();
extern void isr43();
extern void isr44();
extern void isr45();
extern void isr46();
extern void isr47();

extern void isr128();
extern void isr129();
extern void isr255();

#define IRQ0  32
#define IRQ1  33
#define IRQ2  34
#define IRQ3  35
#define IRQ4  36
#define IRQ5  37
#define IRQ6  38
#define IRQ7  39
#define IRQ8  40
#define IRQ9  41
#define IRQ10 42
#define IRQ11 43
#define IRQ12 44
#define IRQ13 45
#define IRQ14 46
#define IRQ15 47

#define PIC1_LOCATION 0x20 /* Master PIC Address */
#define PIC2_LOCATION 0xA0 /* Slave PIC Address */
#define PIC1_COMMAND (PIC1_LOCATION+0) /* Master Command */
#define PIC2_COMMAND (PIC2_LOCATION+0) /* Slave Command */
#define PIC1_DATA (PIC1_LOCATION+1) /* Master Data */
#define PIC2_DATA (PIC2_LOCATION+1) /* Slave Data */
#define PIC_REMAP_BEG_OFFSET (PIC1_LOCATION+0) /* Remap Beginning Location */
#define PIC_REMAP_END_OFFSET (PIC1_LOCATION+8) /* Remap Ending Location */

#define PIC_EOI_MASTER 256
#define PIC_EOI_SLAVE (PIC_EOI_MASTER+1)
#define PIC_EOI_ALL (PIC_EOI_SLAVE +1)

typedef struct registers{
    uint64_t r15, r14, r13, r12, r11, r10, r9, r8, rbp, rcx, rbx;
    uint64_t rdx, rsi, rdi;
    uint64_t es, ds, fs, gs, rax;
    uint64_t type, ecode;
    uint64_t rip, cs, rflags, uesp, ss;
} registers_t ;

/** Register callback function */
typedef void (*isr_t)(uint64_t error_code, registers_t*);

/**
 * Registers ISR function for interrupt.
 */
void register_interrupt_handler(uint8_t interrupt_id, isr_t handler_func);
