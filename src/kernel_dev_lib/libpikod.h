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
 * libpikod.h
 *  Created on: Dec 27, 2015
 *      Author: Peter Vanusanik
 *  Contents: developer needed static library functions for processes such as init, daemons, daemon loaders etc
 *  	Should only be used to create those.
 */

#pragma once

#ifdef __cplusplus
extern "C" {
#endif

extern uint64_t dev_sys_0arg(uint64_t syscallnum);
extern uint64_t dev_sys_1arg(uint64_t syscallnum, void* arg1);
extern uint64_t dev_sys_2arg(uint64_t syscallnum, void* arg1, void* arg2);
extern uint64_t dev_sys_3arg(uint64_t syscallnum, void* arg1, void* arg2, void* arg3);
extern uint64_t dev_sys_4arg(uint64_t syscallnum, void* arg1, void* arg2, void* arg3, void* arg4);
extern uint64_t dev_sys_5arg(uint64_t syscallnum, void* arg1, void* arg2, void* arg3, void* arg4, void* arg5);

#ifdef __cplusplus
}
#endif
