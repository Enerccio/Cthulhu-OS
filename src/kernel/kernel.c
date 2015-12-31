/*
 * The MIT License (MIT)
 * Copyright (c) 2015 Peter Vanusanik <admin@en-circle.com>
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy of
 * this software and associated documentation files (the "Software"), to deal in the
 * Software without restriction, including without limitation the rights to use, copy,
 * modify, merge, publish, distribute, sublicense, and/or sell copies of the Software,
 * and to permit persons to whom the Software is furnished to do so, subject to the
 * following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all copies
 * or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED,
 * INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR
 * PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE
 * LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
 * TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE
 * OR OTHER DEALINGS IN THE SOFTWARE.
 *
 * kernel.c
 *  Created on: Dec 22, 2015
 *      Author: Peter Vanusanik
 *  Contents: main kernel entry function (in long mode in c).
 */
#include "ports/ports.h"
#include "commons.h"
#include "utils/rsod.h"
#include "utils/collections/array.h"
#include "memory/heap.h"
#include "memory/paging.h"
#include "structures/acpi.h"
#include "syscalls/sys.h"
#include <stdlib.h>
#include <stdio.h>
#include <sys/kernel.h>

#include "cpus/cpu_mgmt.h"
#include "cpus/ipi.h"
#include "interrupts/clock.h"
#include "interrupts/idt.h"
#include "interrupts/interrupts.h"
#include "structures/gdt.h"
#include "rlyeh/rlyeh.h"

extern volatile uint64_t clock_ms;
extern struct multiboot_info multiboot_info;

uint64_t kernel_tmp_heap_start;

extern bool __ports_initialized;
extern bool __print_initialized;

void kernel_main(struct multiboot_info* mboot_addr, uint64_t heap_start) {
	__ports_initialized = false;
	__print_initialized = false;
	cpus = NULL;
	kernel_tmp_heap_start = heap_start;

	initialize_temporary_heap(heap_start);
	initialize_physical_memory_allocation(mboot_addr);
	initialize_standard_heap();

	mboot_addr = &multiboot_info;

	initialize_logger();
	log_msg("Paging memory and kernel heap initialized");
	log_msg("Logger initialized");

	initialize_ports();
	log_msg("Ports initialized");

	__initialize_kclib();
	log_msg("KCLib initialized");

	initialize_grx(mboot_addr);

    kd_cclear(0);
    kd_cwrite("Azathoth kernel bootup sequence initiated\n", 0, 5);

    init_errors();

    log_msg("Graphical mode initialized");
    log_msg("Errors initialized");

    init_table_acpi();
    vlog_msg("ACPI tables queried and ready to use (if found).");

    initialize_cpus();
    log_msg("CPU status queried.");

    reinitialize_gdt();
    vlog_msg("GDT reinitialized to %xh", (uint64_t)&gdt);

    initialize_interrupts();
    log_msg("Interrupt table initialized");

    register_standard_interrupt_handlers();
    log_msg("Preliminary interrupt handlers set up");

    initialize_clock();
    vlog_msg("Kernel clock initialized, current time in unix time %llu", get_unix_time());

    if (array_get_size(cpus) > 1) {
		initialize_mp(get_local_apic_id());
	}
    vlog_msg("CPU queried and initialized. Number of logical cpus %u", array_get_size(cpus));

    initialize_ipi_subsystem();
	initialize_lapic();
	log_msg("Inter-processor interrupts initialized");

    deallocate_start_memory();
    log_msg("Bootup memory removed.");

    init_initramfs(mboot_addr);
    log_msg("Initramfs loaded");

    initialize_system_calls();
    log_msg("System calls initialized");

    initialize_userspace();
}
