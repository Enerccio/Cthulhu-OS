/*
 * The MIT License (MIT)
 * Copyright (c) 2015 Peter Vanusanik <admin@en-circle.com>
 *
 * Permission is hereby granted, free of charge, to any person obtaining
 * a copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation the
 * rights to use, copy, modify, merge, publish, distribute, sublicense, and/or
 * sell copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included
 * in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED,
 * INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR
 * PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE
 * LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
 * TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE
 * USE OR OTHER DEALINGS IN THE SOFTWARE.
 *
 * rsod.h
 *  Created on: Dec 22, 2015
 *      Author: Peter Vanusanik
 *  Contents: nonrecoverable error presentation screen
 */
#pragma once

#include "../commons.h"

#define UNDEFINED_ERROR                           0
#define ERROR_NO_MEMORY_DETECTED                  1
#define ERROR_INTERNAL_LIBC                       2
#define ERROR_NO_IV_FOR_INTERRUPT                 3
#define ERROR_KERNEL_UNHANDLED_DB_EXCEPTION       4
#define ERROR_KERNEL_DIVISION_BY_ZERO             5
#define ERROR_KERNEL_UNHANDLED_NMI_INTERRUPT      6
#define ERROR_KERNEL_BREAKPOINT_TRAP              7
#define ERROR_KERNEL_OVERFLOW_ERROR               8
#define ERROR_KERNEL_BRANGE_EXCEEDED_EXCEPTION    9
#define ERROR_KERNEL_INVALID_OPCODE_EXCEPTION    10
#define ERROR_KERNEL_DEVICE_NOT_AVAILABLE_EXC    11
#define ERROR_KERNEL_UNHANDLED_DOUBLE_FAULT      12
#define ERROR_KERNEL_INVALID_TSS                 13
#define ERROR_KERNEL_SEGMENT_NOT_PRESENT         14
#define ERROR_KERNEL_STACK_FAULT_EXCEPTION       15
#define ERROR_KERNEL_GENERAL_PROTECTION_EXC      16
#define ERROR_KERNEL_PAGE_FAULT_IN_NONPAGED_AREA 17
#define ERROR_KERNEL_FPU_ERROR                   18
#define ERROR_KERNEL_ALIGN_CHECK_EXCEPTION       19
#define ERROR_KERNEL_MACHINE_CHECK_EXCEPTION     20
#define ERROR_KERNEL_SIMD_FP_EXCEPTION           21
#define ERROR_KERNEL_VIRTUALIZATION_EXCEPTION    22
#define ERROR_MINIMAL_MEMORY_FAILURE             23
#define ERROR_KERNEL_IPI_EXCEPTION               24
#define ERROR_NO_FONT_DETECTED                   25
#define ERROR_INITRD_ERROR                       26

/**
 * Initializes error subsystem.
 */
void init_errors();

/**
 * Shows rainbow screen of the death. Halts all processors.
 */
void error(uint16_t ecode, ruint_t speccode, ruint_t speccode2, void* eaddress);
