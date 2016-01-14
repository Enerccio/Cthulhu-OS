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
 * sys.h
 *  Created on: Dec 27, 2015
 *      Author: Peter Vanusanik
 *  Contents: syscall operation
 */

#pragma once

#include "../commons.h"
#include "../interrupts/idt.h"

typedef ruint_t (*syscall_0)(registers_t*);
typedef ruint_t (*syscall_1)(registers_t*, ruint_t);
typedef ruint_t (*syscall_2)(registers_t*, ruint_t, ruint_t);
typedef ruint_t (*syscall_3)(registers_t*, ruint_t, ruint_t, ruint_t);
typedef ruint_t (*syscall_4)(registers_t*, ruint_t, ruint_t, ruint_t, ruint_t);
typedef ruint_t (*syscall_5)(registers_t*, ruint_t, ruint_t, ruint_t, ruint_t, ruint_t);
typedef struct syscall {
    bool present;
    bool uses_error;
    bool unsafe;
    uint8_t args;
    union {
        syscall_0 _0;
        syscall_1 _1;
        syscall_2 _2;
        syscall_3 _3;
        syscall_4 _4;
        syscall_5 _5;
    } syscall;
} syscall_t;

syscall_t make_syscall_0(syscall_0 sfnc, bool e, bool unsafe);
syscall_t make_syscall_1(syscall_1 sfnc, bool e, bool unsafe);
syscall_t make_syscall_2(syscall_2 sfnc, bool e, bool unsafe);
syscall_t make_syscall_3(syscall_3 sfnc, bool e, bool unsafe);
syscall_t make_syscall_4(syscall_4 sfnc, bool e, bool unsafe);
syscall_t make_syscall_5(syscall_5 sfnc, bool e, bool unsafe);

void register_syscall(bool system, uint8_t syscall_id, syscall_t syscall);

void initialize_system_calls();
