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
 * gdt.c
 *  Created on: Dec 28, 2015
 *      Author: Peter Vanusanik
 *  Contents: 
 */

#include "gdt.h"
#include "../cpus/cpu_mgmt.h"
#include "../cpus/ipi.h"
#include "../interrupts/idt.h"

gdt_ptr_t gdt;

extern void load_gdt(gdt_ptr_t* gdt, uint16_t tssid);

void reinitialize_gdt() {
    uint16_t numdesc = 6+(3*array_get_size(cpus));
    descriptor_t* dscp = malloc(sizeof(descriptor_t)*numdesc);
    gdt.descriptors = dscp;
    gdt.limit = (sizeof(descriptor_t)*numdesc)-1;

    memset(gdt.descriptors, 0, sizeof(descriptor_t)*numdesc);

    descriptor_t* kernel_code = &gdt.descriptors[1];
    kernel_code->s = 1;
    kernel_code->type = 10 & 0xF;
    kernel_code->p = 1;
    kernel_code->l = 1;

    descriptor_t* kernel_data = &gdt.descriptors[2];
    kernel_data->s = 1;
    kernel_data->type = 2 & 0xF;
    kernel_data->p = 1;
    kernel_data->dpl = 0;

    // 3rd isn't used

    descriptor_t* u_data = &gdt.descriptors[4];
    u_data->s = 1;
    u_data->type = 2 & 0xF;
    u_data->p = 1;
    u_data->dpl = 3;

    descriptor_t* u_code = &gdt.descriptors[5];
    u_code->s = 1;
    u_code->type = 10 & 0xF;
    u_code->dpl = 3;
    u_code->p = 1;
    u_code->l = 1;

    tss_descriptor_t* tsd;
    size_t asize = array_get_size(cpus);
    int itc = 6;
    for (size_t i=0; i<asize; i++) {
        cpu_t* cpu = array_get_at(cpus, i);
        tsd = (tss_descriptor_t*)&gdt.descriptors[itc];
        itc += 2;
        cputss_t* tss = malloc(sizeof(cputss_t));
        memset(tss, 0, sizeof(cputss_t));

        tss->rsp0 = (uintptr_t)cpu->stack;
        tss->ist1 = (uintptr_t)cpu->handler_stack;
        tss->ist2 = (uintptr_t)cpu->pf_stack;
        tss->ist3 = (uintptr_t)cpu->df_stack;
        tss->ist4 = (uintptr_t)cpu->ipi_stack;

        uintptr_t tss_point = (uintptr_t)tss;
        tsd->descriptor.base0015 = tss_point & 0xFFFF;
        tsd->descriptor.base2316 = (tss_point >> 16) & 0xFF;
        tsd->descriptor.base3124 = (tss_point >> 24) & 0xFF;
        tsd->base6332 = (tss_point >> 32) & 0xFFFFFFFF;
        tsd->descriptor.limit0015 = sizeof(cputss_t);
        tsd->descriptor.s = 0;
        tsd->descriptor.dpl = 0;
        tsd->descriptor.g = 0;
        tsd->descriptor.type = 9;
        tsd->descriptor.p = 1;

        descriptor_t* gs = &gdt.descriptors[itc];
        gs->s = 1;
        gs->type = 2 & 0xF;
        gs->p = 1;
        gs->dpl = 3;
    }

    load_gdt(&gdt, 48);

    uint8_t localcpu = get_local_apic_id();
    uint32_t proclen = array_get_size(cpus);
    for (uint32_t i=0; i<proclen; i++) {
        cpu_t* cpu = array_get_at(cpus, i);
        if (cpu->apic_id == localcpu) {
            write_gs((ruint_t)cpu);
            return;
        }
    }
}
