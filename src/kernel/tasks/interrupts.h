/*
 * The MIT License (MIT)
 * Copyright (c) 2015 Peter Vanusanik <admin@en-circle.com>
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies
 * of the Software, and to permit persons to whom the Software is furnished to do so,
 * subject to the following conditions:
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
 * interrupts.h
 *  Created on: Dec 23, 2015
 *      Author: Peter Vanusanik
 *  Contents: Interrupt handler definitions
 */

#pragma once
#include "idt.h"

#define EXC_DE  0
#define EXC_DB  1
#define EXC_NMI 2
#define EXC_BP  3
#define EXC_OF  4
#define EXC_BR  5
#define EXC_UD  6
#define EXC_NM  7
#define EXC_DF  8
#define EXC_TS  10
#define EXC_NP  11
#define EXC_SS  12
#define EXC_GP  13
#define EXC_PF  14
#define EXC_MF  16
#define EXC_AC  17
#define EXC_MC  18
#define EXC_XM  19
#define EXC_VE  20

void register_standard_interrupt_handlers();
