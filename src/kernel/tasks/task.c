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
 *  Created on: Dec 23, 2015
 *      Author: Peter Vanusanik
 *  Contents: task management
 */

#include "task.h"

array_t* cpus;
extern volatile uintmax_t clock_ms;
extern volatile uintmax_t clock_s;
uint32_t cpuid_to_cputord[256];
uint32_t apicaddr;

void ap_main(uint64_t proc_id){
	cpu_t* cpu = (cpu_t*)array_get_at(cpus, cpuid_to_cputord[proc_id]);
	cpu->started = true;

	bool ddebug = true;
	while (ddebug) ;

	while (true) ;
}

void send_ipi_to(uint8_t apic_id, uint8_t vector) {
	while(*(bool*)physical_to_virtual((apicaddr + 0x30) & (1<<12)))
    	;

    uint32_t sendvalue = ((uint32_t)apic_id) << 24;
    memcpy((void*)physical_to_virtual(apicaddr + 0x31 * 0x10), &sendvalue, sizeof(uint32_t));

    uint32_t control = vector;

    /* required to be set for non-INIT IPIs */
    control |= 1<<14;

    /* otherwise, everything's good: IPI mode, fixed delivery vector,
        only to the specified APIC, edge-triggered. */
    sendvalue = control;
    memcpy((void*)physical_to_virtual(apicaddr + 0x30 * 0x10), &sendvalue, sizeof(uint32_t));
}

void initialize_mp(unsigned int localcpu){
	uint32_t proclen = array_get_size(cpus);
	uintmax_t clock_data;
	uintmax_t clock_sdata;

	for (uint32_t i=0; i<proclen; i++){
		cpu_t* cpu = array_get_at(cpus, i);
		if (cpu->processor_id == localcpu){
			cpu->started = true;
			continue;
		}
		vlog_msg("Attempting to initialize logical cpu %llu, apic %hhx.", cpu->processor_id, cpu->apic_id);

		// INIT IPI
		uint32_t sendvalue = ((uint32_t)cpu->apic_id) << 24;
		memcpy((void*)physical_to_virtual(apicaddr + 0x31 * 0x10), &sendvalue, sizeof(uint32_t));
		sendvalue = 5 << 7;
		memcpy((void*)physical_to_virtual(apicaddr + 0x30 * 0x10), &sendvalue, sizeof(uint32_t));

		clock_data = clock_ms+10;
		clock_sdata = clock_s;
		while (clock_data <= clock_ms && clock_sdata != clock_s) ;

		// SIPI
		sendvalue = ((uint32_t)cpu->apic_id) << 24;
		memcpy((void*)physical_to_virtual(apicaddr + 0x31 * 0x10), &sendvalue, sizeof(uint32_t));
		sendvalue = (6 << 7) | 8;
		memcpy((void*)physical_to_virtual(apicaddr + 0x30 * 0x10), &sendvalue, sizeof(uint32_t));
	}

	clock_data = clock_ms+200;
	clock_sdata = clock_s;
	while (clock_data <= clock_ms && clock_sdata != clock_s) ;

	bool initialized = true;
	for (uint32_t i=0; i<proclen; i++){
		cpu_t* cpu = array_get_at(cpus, i);
		if (!cpu->started){
			initialized = false;
			break;
		}
	}

	if (!initialized){
		for (uint32_t i=0; i<proclen; i++){
			cpu_t* cpu = array_get_at(cpus, i);
			if (cpu->processor_id == localcpu){
				continue;
			}
			// Second SIPI
			uint32_t sendvalue = ((uint32_t)cpu->apic_id) << 24;
			memcpy((void*)physical_to_virtual(apicaddr + 0x31 * 0x10), &sendvalue, sizeof(uint32_t));
			sendvalue = (6 << 7) | 8;
			memcpy((void*)physical_to_virtual(apicaddr + 0x30 * 0x10), &sendvalue, sizeof(uint32_t));
		}
	}
}

cpu_t* make_cpu(MADT_LOCAL_APIC* apic) {
	cpu_t* cpu = malloc(sizeof(cpu_t));
	if (cpu == NULL)
		error(ERROR_MINIMAL_MEMORY_FAILURE, 0, 0, &make_cpu);
	cpu->processor_id = apic->processor_id;
	cpu->apic_id = apic->id;
	cpu->started = false;
	cpu->processes = create_array();
	cpu->stack = (void*) (((uintptr_t)malloc(0x2000))+0x2000);
	if (cpu->processes == NULL)
		error(ERROR_MINIMAL_MEMORY_FAILURE, 0, 0, &make_cpu);
	return cpu;
}

cpu_t* make_cpu_default() {
	cpu_t* cpu = malloc(sizeof(cpu_t));
	if (cpu == NULL)
		error(ERROR_MINIMAL_MEMORY_FAILURE, 0, 0, &make_cpu);
	cpu->processor_id = 0;
	cpu->started = false;
	cpu->processes = create_array();
	cpu->stack = (void*) (((uintptr_t)malloc(0x2000))+0x2000);
	if (cpu->processes == NULL)
		error(ERROR_MINIMAL_MEMORY_FAILURE, 0, 0, &make_cpu);
	return cpu;
}

void initialize_cpus() {

	memset(cpuid_to_cputord, 0, sizeof(cpuid_to_cputord));

	cpus = create_array_spec(256);
	unsigned int cnt = 0;
	unsigned int localcpu = 0;
	MADT_HEADER* madt = find_madt();

	if (madt == NULL){
		// no acpi, use single cpu only
		cpuid_to_cputord[0] = 0;
		array_push_data(cpus, make_cpu_default());
	} else {
		MADT_LOCAL_APIC* localapic = (MADT_LOCAL_APIC*)physical_to_virtual((uint64_t)madt->address);
		apicaddr = madt->address;
		vlog_msg("Local apic address %#X.", apicaddr);
		localcpu = localapic->id;

		size_t bytes = madt->header.Length;
		bytes -= sizeof(MADT_HEADER);
		uint64_t addr = ((uint64_t)madt)+sizeof(MADT_HEADER);
		while (bytes > 0){
			ACPI_SUBTABLE_HEADER* h = (ACPI_SUBTABLE_HEADER*)addr;
			bytes -= h->length;
			addr += h->length;
			if (h->type == ACPI_MADT_TYPE_LOCAL_APIC){
				MADT_LOCAL_APIC* lapic = (MADT_LOCAL_APIC*)h;
				if (lapic->lapic_flags == 1){
					array_push_data(cpus, make_cpu(lapic));
					cpuid_to_cputord[lapic->processor_id] = array_get_size(cpus)-1;
					++cnt;
				}
			}
		}
	}

	if (array_get_size(cpus) > 1)
		initialize_mp(localcpu);
}

void initialize_kernel_task() {

}
