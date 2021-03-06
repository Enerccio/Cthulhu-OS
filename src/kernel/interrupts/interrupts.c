/*
 * The MIT License (MIT)
 * Copyright (c) 2015 Peter Vanusanik <admin@en-circle.com>
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is furnished
 * to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED,
 * INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR
 * PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE
 * FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR
 * OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 *
 * interrupts.c
 *  Created on: Dec 23, 2015
 *      Author: Peter Vanusanik
 *  Contents: Interrupt handler definitions
 */

#include "../utils/rsod.h"

#include <stdio.h>
#include "../interrupts/interrupts.h"
#include "../memory/paging.h"
#include "../cpus/cpu_mgmt.h"

extern void* get_faulting_address();
extern void kp_halt();

void de_exception(ruint_t ecode, registers_t* registers) {
    error(ERROR_KERNEL_DIVISION_BY_ZERO, registers->rip, registers->cs, registers);
}

void db_exception(ruint_t ecode, registers_t* registers) {
    error(ERROR_KERNEL_UNHANDLED_DB_EXCEPTION, registers->rip, registers->cs, registers);
}

void nmi_exception(ruint_t ecode, registers_t* registers) {
    error(ERROR_KERNEL_UNHANDLED_NMI_INTERRUPT, registers->rip, registers->cs, registers);
}

void bp_exception(ruint_t ecode, registers_t* registers) {
    error(ERROR_KERNEL_BREAKPOINT_TRAP, registers->rip, registers->cs, registers);
}

void of_exception(ruint_t ecode, registers_t* registers) {
    error(ERROR_KERNEL_OVERFLOW_ERROR, registers->rip, registers->cs, registers);
}

void br_exception(ruint_t ecode, registers_t* registers) {
    error(ERROR_KERNEL_BRANGE_EXCEEDED_EXCEPTION, registers->rip, registers->cs, registers);
}

void ud_exception(ruint_t ecode, registers_t* registers) {
    error(ERROR_KERNEL_INVALID_OPCODE_EXCEPTION, registers->rip, registers->cs, registers);
}

void nm_exception(ruint_t ecode, registers_t* registers) {
    error(ERROR_KERNEL_DEVICE_NOT_AVAILABLE_EXC, registers->rip, registers->cs, registers);
}

void df_exception(ruint_t ecode, registers_t* registers) {
    error(ERROR_KERNEL_UNHANDLED_DOUBLE_FAULT, registers->rip, registers->cs, registers);
}

void ts_exception(ruint_t ecode, registers_t* registers) {
    error(ERROR_KERNEL_INVALID_TSS, registers->rip, registers->cs, registers);
}

void np_exception(ruint_t ecode, registers_t* registers) {
    error(ERROR_KERNEL_SEGMENT_NOT_PRESENT, registers->rip, ecode, registers);
}

void ss_exception(ruint_t ecode, registers_t* registers) {
    error(ERROR_KERNEL_STACK_FAULT_EXCEPTION, registers->rip, ecode, registers);
}

void gp_exception(ruint_t ecode, registers_t* registers) {
    //error(ERROR_KERNEL_GENERAL_PROTECTION_EXC, registers->rip, ecode, registers);
    debug_break;
}

void pf_exception(ruint_t ecode, registers_t* registers) {
    void* fa = get_faulting_address();
    if (registers->cs == 0x8) {
        // kernel page fault
        // TODO: add swap
        debug_break;
        return;
        // error(ERROR_KERNEL_PAGE_FAULT_IN_NONPAGED_AREA, (ruint_t)fa, ecode, (void*)registers->rip);
    } else {
        if (!page_fault((uintptr_t)fa, registers->ecode)) {
            // TODO: add abort
            debug_break;
        }
    }
}

void mf_exception(ruint_t ecode, registers_t* registers) {
    error(ERROR_KERNEL_FPU_ERROR, registers->rip, registers->cs, registers);
}

void ac_exception(ruint_t ecode, registers_t* registers) {
    error(ERROR_KERNEL_ALIGN_CHECK_EXCEPTION, registers->rip, ecode, registers);
}

void mc_exception(ruint_t ecode, registers_t* registers) {
    error(ERROR_KERNEL_MACHINE_CHECK_EXCEPTION, registers->rip, registers->cs, registers);
}

void xm_exception(ruint_t ecode, registers_t* registers) {
    error(ERROR_KERNEL_SIMD_FP_EXCEPTION, registers->rip, registers->cs, registers);
}

void ve_exception(ruint_t ecode, registers_t* registers) {
    error(ERROR_KERNEL_VIRTUALIZATION_EXCEPTION, registers->rip, registers->cs, registers);
}

/**
 * Registers standard interrupt handlers.
 *
 * Maps all handlers for all standard interrupts.
 */
void register_standard_interrupt_handlers() {
    register_interrupt_handler(EXC_DE, de_exception);
    register_interrupt_handler(EXC_DB, db_exception);
    register_interrupt_handler(EXC_NMI, nmi_exception);
    register_interrupt_handler(EXC_BP, bp_exception);
    register_interrupt_handler(EXC_OF, of_exception);
    register_interrupt_handler(EXC_BR, br_exception);
    register_interrupt_handler(EXC_UD, ud_exception);
    register_interrupt_handler(EXC_NM, nm_exception);
    register_interrupt_handler(EXC_DF, df_exception);
    register_interrupt_handler(EXC_TS, ts_exception);
    register_interrupt_handler(EXC_NP, np_exception);
    register_interrupt_handler(EXC_SS, ss_exception);
    register_interrupt_handler(EXC_GP, gp_exception);
    register_interrupt_handler(EXC_PF, pf_exception);
    register_interrupt_handler(EXC_MF, mf_exception);
    register_interrupt_handler(EXC_AC, ac_exception);
    register_interrupt_handler(EXC_MC, mc_exception);
    register_interrupt_handler(EXC_XM, xm_exception);
    register_interrupt_handler(EXC_VE, ve_exception);
}
