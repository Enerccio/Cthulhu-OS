/*
 * The MIT License (MIT)
 * Copyright (c) 2015 Peter Vanusanik <admin@en-circle.com>
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the Software
 * is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED,
 * INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR
 * PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE
 * FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR
 * OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 *
 * rsod.c
 *  Created on: Dec 22, 2015
 *      Author: Peter Vanusanik
 *  Contents: nonrecoverable error presentation screen
 */
#include "rsod.h"

#include "../cpus/cpu_mgmt.h"
#include "../cpus/ipi.h"
#include "../utils/kstdlib.h"

const char* top_message = "We are sorry but kernel has suffered an error";
const char* top_message_2 = "from which it was unable to recover->";
const char* bottom_message = "Please check recent kernel updates or";
const char* bottom_message_2 = "hardware changes which can be the fault here.";
char* error_codes[8000];

extern void kp_halt();

#define BACKGROUND_COLOR 0
#define FOREGROUND_COLOR fgc(&fg)

static uint64_t lock;
extern void proc_spinlock_lock(void* address);
extern void proc_spinlock_unlock(void* address);

/**
 * Initializes error subsystem.
 *
 * Fills error message array.
 */
void init_errors() {
    log_msg("Initializing error page");

    for (uint16_t i = 0; i < 8000; i++) {
        error_codes[i] = "";
    }

    error_codes[UNDEFINED_ERROR] = "UNDEFINED_ERROR";
    error_codes[ERROR_NO_MEMORY_DETECTED] = "ERROR_NO_MEMORY_DETECTED";
    error_codes[ERROR_INTERNAL_LIBC] = "ERROR_INTERNAL_LIBC";
    error_codes[ERROR_NO_IV_FOR_INTERRUPT] = "ERROR_NO_IV_FOR_INTERRUPT";

    error_codes[ERROR_KERNEL_UNHANDLED_DB_EXCEPTION] = "ERROR_KERNEL_UNHANDLED_DB_EXCEPTION";
    error_codes[ERROR_KERNEL_DIVISION_BY_ZERO] = "ERROR_KERNEL_DIVISION_BY_ZERO";
    error_codes[ERROR_KERNEL_UNHANDLED_NMI_INTERRUPT] = "ERROR_KERNEL_UNHANDLED_NMI_INTERRUPT";
    error_codes[ERROR_KERNEL_BREAKPOINT_TRAP] = "ERROR_KERNEL_BREAKPOINT_TRAP";
    error_codes[ERROR_KERNEL_OVERFLOW_ERROR] = "ERROR_KERNEL_OVERFLOW_ERROR";
    error_codes[ERROR_KERNEL_BRANGE_EXCEEDED_EXCEPTION] = "ERROR_KERNEL_BRANGE_EXCEEDED_EXCEPTION";
    error_codes[ERROR_KERNEL_INVALID_OPCODE_EXCEPTION] = "ERROR_KERNEL_INVALID_OPCODE_EXCEPTION";
    error_codes[ERROR_KERNEL_DEVICE_NOT_AVAILABLE_EXC] = "ERROR_KERNEL_DEVICE_NOT_AVAILABLE_EXC";
    error_codes[ERROR_KERNEL_UNHANDLED_DOUBLE_FAULT] = "ERROR_KERNEL_UNHANDLED_DOUBLE_FAULT";
    error_codes[ERROR_KERNEL_INVALID_TSS] = "ERROR_KERNEL_INVALID_TSS";
    error_codes[ERROR_KERNEL_SEGMENT_NOT_PRESENT] = "ERROR_KERNEL_SEGMENT_NOT_PRESENT";
    error_codes[ERROR_KERNEL_STACK_FAULT_EXCEPTION] = "ERROR_KERNEL_STACK_FAULT_EXCEPTION";
    error_codes[ERROR_KERNEL_GENERAL_PROTECTION_EXC] = "ERROR_KERNEL_GENERAL_PROTECTION_EXC";
    error_codes[ERROR_KERNEL_PAGE_FAULT_IN_NONPAGED_AREA] = "ERROR_KERNEL_PAGE_FAULT_IN_NONPAGED_AREA";
    error_codes[ERROR_KERNEL_FPU_ERROR] = "ERROR_KERNEL_FPU_ERROR";
    error_codes[ERROR_KERNEL_ALIGN_CHECK_EXCEPTION] = "ERROR_KERNEL_ALIGN_CHECK_EXCEPTION";
    error_codes[ERROR_KERNEL_MACHINE_CHECK_EXCEPTION] = "ERROR_KERNEL_MACHINE_CHECK_EXCEPTION";
    error_codes[ERROR_KERNEL_SIMD_FP_EXCEPTION] = "ERROR_KERNEL_SIMD_FP_EXCEPTION";
    error_codes[ERROR_KERNEL_VIRTUALIZATION_EXCEPTION] = "ERROR_KERNEL_VIRTUALIZATION_EXCEPTION";
    error_codes[ERROR_MINIMAL_MEMORY_FAILURE] = "ERROR_MINIMAL_MEMORY_FAILURE";
    error_codes[ERROR_KERNEL_IPI_EXCEPTION] = "ERROR_KERNEL_IPI_EXCEPTION";
    error_codes[ERROR_NO_FONT_DETECTED] = "ERROR_NO_FONT_DETECTED";

    lock = 0;
}

/**
 * Returns random foreground color.
 */
uint8_t fgc(uint8_t* fgc) {
    uint8_t fg = *fgc;
    while (*fgc == fg) {
        *fgc = ((*fgc + rand_number(16)) % 16);
        if (*fgc == 0)
            *fgc = 1;
    }
    return fg;
}

static volatile bool error_displaying = 0;
/**
 * Shows rainbow screen of the death. Halts all processors.
 *
 * If two concurrent attempts to display error code, only displays first and then halt
 */
void error(uint16_t ecode, uint64_t speccode, uint64_t speccode2, void* eaddress) {

	proc_spinlock_lock(&lock);
	if (error_displaying != 0) {
		proc_spinlock_unlock(&lock);
		cpu_t* ccput = get_current_cput();
		ccput->started = false;
		kp_halt();
	} else {
		error_displaying = 1;
	}
	proc_spinlock_unlock(&lock);

    uint8_t fg = rand_number(15)+1;

    kd_cclear(BACKGROUND_COLOR);

    kd_setxy(10, 2);
    kd_cwrite(top_message, BACKGROUND_COLOR, FOREGROUND_COLOR);
    kd_setxy(10, 3);
    kd_cwrite(top_message_2, BACKGROUND_COLOR, FOREGROUND_COLOR);

    kd_setxy(16, 5);
    kd_cwrite("Error code:         ", BACKGROUND_COLOR, FOREGROUND_COLOR);
    kd_cwrite_hex64(ecode, BACKGROUND_COLOR, FOREGROUND_COLOR);

    kd_setxy(16, 6);
    kd_cwrite(error_codes[ecode], BACKGROUND_COLOR, FOREGROUND_COLOR);

    kd_setxy(16, 7);
    kd_cwrite("Specific code:      ", BACKGROUND_COLOR, FOREGROUND_COLOR);
    kd_cwrite_hex64(speccode, BACKGROUND_COLOR, FOREGROUND_COLOR);

    kd_setxy(16, 8);
    kd_cwrite("Additional code:    ", BACKGROUND_COLOR, FOREGROUND_COLOR);
    kd_cwrite_hex64(speccode2, BACKGROUND_COLOR, FOREGROUND_COLOR);

    kd_setxy(16, 10);
    kd_cwrite("Address:            ", BACKGROUND_COLOR, FOREGROUND_COLOR);
    kd_cwrite_hex64((uint64_t) eaddress, BACKGROUND_COLOR, FOREGROUND_COLOR);

    if (cpus != NULL) {
		kd_setxy(16, 12);
		kd_cwrite("Faulting processor: ", BACKGROUND_COLOR, FOREGROUND_COLOR);
		kd_cwrite_hex64(get_local_processor_id(), BACKGROUND_COLOR, FOREGROUND_COLOR);
    }

    kd_setxy(10, 19);
    kd_cwrite(bottom_message, BACKGROUND_COLOR, FOREGROUND_COLOR);
    kd_setxy(10, 20);
    kd_cwrite(bottom_message_2, BACKGROUND_COLOR, FOREGROUND_COLOR);

    if (cpus == NULL)
    	kp_halt();
    else {
    	broadcast_ipi_message(IPI_HALT_IMMEDIATELLY, ecode+1, 0);
    }

    while (true) ;
}

