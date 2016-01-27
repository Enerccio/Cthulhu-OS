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
 * cpu_mgmt.c
 *  Created on: Dec 23, 2015
 *      Author: Peter Vanusanik
 *  Contents: cpu management
 */

#include "cpu_mgmt.h"

#include "ipi.h"
#include "../structures/gdt.h"
#include "../interrupts/clock.h"
#include "../interrupts/idt.h"
#include "../ports/ports.h"
#include "../processes/scheduler.h"
#include "../processes/process.h"

extern volatile uintmax_t clock_ms;
extern volatile uintmax_t clock_s;
extern void idt_flush(void* addr);
extern idt_ptr_t idt_ptr;
extern void* Gdt32;
extern void wait_until_activated(ruint_t wait_code);
extern void load_gdt(gdt_ptr_t* gdt, uint16_t tssid);
extern gdt_ptr_t gdt;
extern void kp_halt();
extern uintptr_t get_active_page();

#define AP_INIT_LOAD_ADDRESS (2)
#define INIT_IPI_FLAGS (5<<8)
#define SIPI_FLAGS (6<<8)

/** Array of cpu_t structures for all cpus */
array_t* cpus;
/** Maps processor_id into ord in cpus array */
uint32_t cpuid_to_cputord[256];
/** Contains LAPIC address from ACPI */
uint32_t apicaddr;

bool multiprocessing_ready = false;

#define APIC_DESTINATION_FORM_FLAT 0xFFFFFFF

#define APIC_ENABLE_IPI  (0)
#define APIC_DISABLE_IPI (0x0F)

#define APIC_ENABLE_IRQ_IPI(v) ((v) | (1<<8))
#define APIC_DISABLE_IRQ_IPI(v) ((v) & ~(1<<8))

void initialize_lapic() {
    volatile uint32_t* apic_logdest = (uint32_t*)(uintptr_t)physical_to_virtual(apicaddr+0xD0);
    volatile uint32_t* apic_destform = (uint32_t*)(uintptr_t)physical_to_virtual(apicaddr+0xE0);

    *apic_logdest = 1 << 24;
    *apic_destform = APIC_DESTINATION_FORM_FLAT;
    enable_ipi_interrupts();
}

void enable_ipi_interrupts() {
    volatile uint32_t* apic_spurvec = (uint32_t*)(uintptr_t)physical_to_virtual(apicaddr+0xF0);
    volatile uint32_t* apic_taskprior = (uint32_t*)(uintptr_t)physical_to_virtual(apicaddr+0x80);
    *apic_spurvec = APIC_ENABLE_IRQ_IPI(0xFF);
    *apic_taskprior = APIC_ENABLE_IPI;
}

void disable_ipi_interrupts() {
    volatile uint32_t* apic_spurvec = (uint32_t*)(uintptr_t)physical_to_virtual(apicaddr+0xF0);
    volatile uint32_t* apic_taskprior = (uint32_t*)(uintptr_t)physical_to_virtual(apicaddr+0x80);
    *apic_taskprior = APIC_DISABLE_IPI;
    *apic_spurvec = APIC_DISABLE_IRQ_IPI(*apic_spurvec);
}

/**
 * Search predicate for searching by apic.
 */
bool search_for_cpu_by_apic(void* e, void* d) {
    uint8_t cpu_apic_id = (uint8_t)(intptr_t)d;
    cpu_t* cpu = (cpu_t*)e;
    return cpu->apic_id == cpu_apic_id;
}

/**
 * Returns pointer to current cpu's cput structure
 */
cpu_t* get_current_cput() {
    cpu_t* cpu;

    __asm__ __volatile__ (
        "mov %%gs:0, %0"
            : "=r" (cpu)
    );

    return cpu;
}

/**
 * Returns local processor_id from MADT, bound local for every cpu
 */
uint8_t get_local_processor_id() {
    return get_current_cput()->processor_id;
}

/**
 * Returns local apic_id from MADT, bound local for every cpu
 */
uint8_t get_local_apic_id() {
    if (apicaddr == 0)
        return 0;
    return (*((uint32_t*)physical_to_virtual(apicaddr + 0x020))) >> 24;
}

/**
 * Main entry point for AP processors.
 *
 * AP enters here from assembly code in long mode.
 * Then, that processor marks itself as started and
 * loads shared idt table, enabling the interrupts
 * afterwards.
 */
void ap_main(ruint_t proc_id) {
    cpu_t* cpu = (cpu_t*)array_get_at(cpus, cpuid_to_cputord[proc_id]);
    cpu->started = true;

    load_gdt(&gdt, (uint16_t)(cpuid_to_cputord[proc_id]*24)+(48));
    idt_flush(&idt_ptr);

    ENABLE_INTERRUPTS();
    initialize_lapic();

    wait_until_activated(WAIT_SCHEDULER_INIT_WAIT);
    schedule(NULL);

    kp_halt();
}

/**
 * Waits until IPI is free for writing.
 */
void wait_until_ipi_is_free() {
    while((*(uint32_t*)physical_to_virtual((apicaddr + 0x300))) & (1<<12))
        ;
}

/** Writes to address portion of IPI */
#define WRITE_TO_IPI_ADDRESS(address) *((uint32_t*)physical_to_virtual(apicaddr + 0x310)) = ((uint32_t)address) << 24
/** Writes to data portion of IPI */
#define WRITE_TO_IPI_PAYLOAD(payload) *((uint32_t*)physical_to_virtual(apicaddr + 0x300)) = payload

/**
 * Sends interprocessor interrupt to a processor.
 *
 * Processor is identified by apic_id, vector is data sent,
 * control flags and init_ipi decides flags to be sent with.
 *
 * Waits until LAPIC is free, then writes address and data.
 */
void send_ipi_to(uint8_t apic_id, uint8_t vector, uint32_t control_flags, bool init_ipi) {
    wait_until_ipi_is_free();
    uint32_t payload = vector;
    if (!init_ipi)
        payload |= 1<<14;
    payload |= control_flags;
    WRITE_TO_IPI_ADDRESS(apic_id);
    WRITE_TO_IPI_PAYLOAD(payload);
}

/**
 * Initializes multiprocessing.
 *
 * localcpu contains acpi of bsp.
 *
 * Initializes APs by sending INIT IPI to all APs,
 * then waiting 20ms, sending SIPI IPI to all APs,
 * then waits more, and if any APs are still not initialized,
 * sends SIPI IPI again and returns.
 */
void initialize_mp(unsigned int localcpu) {
    uint32_t proclen = array_get_size(cpus);
    for (uint32_t i=0; i<proclen; i++) {
        cpu_t* cpu = array_get_at(cpus, i);
        if (cpu->apic_id == localcpu) {
            cpu->started = true;
            continue;
        }

        send_ipi_to(cpu->apic_id, 0, INIT_IPI_FLAGS, true);
    }

    busy_wait_milis(20);

    for (uint32_t i=0; i<proclen; i++) {
        cpu_t* cpu = array_get_at(cpus, i);
        if (cpu->apic_id == localcpu) {
            cpu->started = true;
            continue;
        }

        send_ipi_to(cpu->apic_id, AP_INIT_LOAD_ADDRESS, SIPI_FLAGS, true);
    }

    busy_wait_milis(200);

    bool initialized = true;
    for (uint32_t i=0; i<proclen; i++) {
        cpu_t* cpu = array_get_at(cpus, i);
        if (!cpu->started) {
            initialized = false;
            break;
        }
    }

    if (!initialized) {
        for (uint32_t i=0; i<proclen; i++) {
            cpu_t* cpu = array_get_at(cpus, i);
            if (cpu->apic_id == localcpu || cpu->started) {
                continue;
            }
            send_ipi_to(cpu->apic_id, AP_INIT_LOAD_ADDRESS, SIPI_FLAGS, true);
        }
    }
}

#define KERNEL_INIT_STACK_SIZE         (0x2000)
#define KERNEL_HANDLER_STACK_SIZE      (0x8000)
#define KERNEL_PF_STACK_SIZE           (0x3000)
#define KERNEL_DF_STACK_SIZE           (0x2000)
#define KERNEL_IPI_STACK_SIZE          (0x3000)
#define KERNEL_SYSCALL_STACK_SIZE      (0x10000)
#define PAGE_ALIGN(x) ((x) & (~(0xFFF)))

static struct chained_element* __thread_queue_get(void* data) {
    return &((thread_t*)data)->schedule_list;
}

/**
 * Creates cpu_t structure from APIC MADT information.
 */
cpu_t* make_cpu(MADT_LOCAL_APIC* apic, size_t insertid) {
    cpu_t* cpu = malloc(sizeof(cpu_t));
    if (cpu == NULL)
        error(ERROR_MINIMAL_MEMORY_FAILURE, 0, 0, &make_cpu);

    if (apic == NULL) {
        cpu->processor_id = 0;
        cpu->apic_id = 0;
        cpu->started = true;
    } else {
        cpu->processor_id = apic->processor_id;
        cpu->apic_id = apic->id;
        cpu->started = false;
    }

    cpu->insert_id = insertid;
    cpu->syscall_stack = (void*) PAGE_ALIGN((uintptr_t)malloc(KERNEL_SYSCALL_STACK_SIZE)
            +KERNEL_SYSCALL_STACK_SIZE);
    cpu->self = cpu;
    cpu->__cpu_lock = 0;
    cpu->__ipi_lock = 0;
    cpu->__cpu_sched_lock = 0;
    cpu->__message_clear_lock = 0;
    cpu->apic_message_handled = 0;
    cpu->current_address_space = get_active_page();
    cpu->stack = (void*) PAGE_ALIGN((uintptr_t)malloc(KERNEL_INIT_STACK_SIZE)+KERNEL_INIT_STACK_SIZE);
    cpu->handler_stack = (void*) PAGE_ALIGN((uintptr_t)malloc(KERNEL_HANDLER_STACK_SIZE)+KERNEL_HANDLER_STACK_SIZE);
    cpu->pf_stack = (void*) PAGE_ALIGN((uintptr_t)malloc(KERNEL_PF_STACK_SIZE)+KERNEL_PF_STACK_SIZE);
    cpu->df_stack = (void*) PAGE_ALIGN((uintptr_t)malloc(KERNEL_DF_STACK_SIZE)+KERNEL_DF_STACK_SIZE);
    cpu->ipi_stack = (void*) PAGE_ALIGN((uintptr_t)malloc(KERNEL_IPI_STACK_SIZE)+KERNEL_IPI_STACK_SIZE);
    cpu->pf_handler.handler = NULL;
    cpu->ct = NULL;
    cpu->priority_0 = create_queue_static(__thread_queue_get);
    cpu->priority_1 = create_queue_static(__thread_queue_get);
    cpu->priority_2 = create_queue_static(__thread_queue_get);
    cpu->priority_3 = create_queue_static(__thread_queue_get);
    cpu->priority_4 = create_queue_static(__thread_queue_get);
    return cpu;
}

/**
 * Creates cpu_t for default cpu, if MADT is unavailable.
 */
cpu_t* make_cpu_default() {
    return make_cpu(NULL, 0);
}

/**
 * Initializes cpu information. Initializes SMP if available.
 *
 * Creates cpus array and fills it in. If SMP is available, initializes
 * all AP processors.
 */
void initialize_cpus() {
    memcpy((void*)physical_to_virtual(0x1000), (void*)physical_to_virtual((uintptr_t)&Gdt32), 0x2000);
    memset(cpuid_to_cputord, 0, sizeof(cpuid_to_cputord));

    apicaddr = 0xFEE00000;

    cpus = create_array_spec(256);
    unsigned int cnt = 0;
    MADT_HEADER* madt = find_madt();

    if (madt == NULL) {
        // no acpi, use single cpu only
        log_warn("No MADT present, only one CPU available.");
        cpuid_to_cputord[0] = 0;
        array_push_data(cpus, make_cpu_default());
    } else {
        apicaddr = madt->address;
        vlog_msg("Local APIC (LAPIC) address: %xh, bootstrap processors apic id: %xh", apicaddr, get_local_apic_id());

        size_t bytes = madt->header.Length;
        bytes -= sizeof(MADT_HEADER);
        uintptr_t addr = ((uintptr_t)madt)+sizeof(MADT_HEADER);
        while (bytes > 0) {
            ACPI_SUBTABLE_HEADER* h = (ACPI_SUBTABLE_HEADER*)addr;
            bytes -= h->length;
            addr += h->length;
            if (h->type == ACPI_MADT_TYPE_LOCAL_APIC) {
                MADT_LOCAL_APIC* lapic = (MADT_LOCAL_APIC*)h;
                if (lapic->lapic_flags == 1) {
                    array_push_data(cpus, make_cpu(lapic, array_get_size(cpus)));
                    cpuid_to_cputord[lapic->processor_id] = array_get_size(cpus)-1;
                    ++cnt;
                }
            }
        }
    }
}
