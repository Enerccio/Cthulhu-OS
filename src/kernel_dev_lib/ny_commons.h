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
 * ny_commons.h
 *  Created on: Jan 2, 2016
 *      Author: Peter Vanusanik
 *  Contents: 
 */

#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#ifndef REGISTER_UINT_TYPE
#define REGISTER_UINT_TYPE uint64_t
#endif
typedef REGISTER_UINT_TYPE ruint_t;

#ifndef PHYSICAL_UINT_TYPE
#define PHYSICAL_UINT_TYPE uintptr_t
#endif
typedef PHYSICAL_UINT_TYPE puint_t;

extern ruint_t dev_sys_0arg(ruint_t syscallnum);
extern ruint_t dev_sys_0arg_e(ruint_t syscallnum, ruint_t* err);
extern ruint_t dev_sys_1arg(ruint_t syscallnum, ruint_t arg1);
extern ruint_t dev_sys_1arg_e(ruint_t syscallnum, ruint_t arg1, ruint_t* err);
extern ruint_t dev_sys_2arg(ruint_t syscallnum, ruint_t arg1, ruint_t arg2);
extern ruint_t dev_sys_2arg_e(ruint_t syscallnum, ruint_t arg1, ruint_t arg2, ruint_t* err);
extern ruint_t dev_sys_3arg(ruint_t syscallnum, ruint_t arg1, ruint_t arg2, ruint_t arg3);
extern ruint_t dev_sys_3arg_e(ruint_t syscallnum, ruint_t arg1, ruint_t arg2, ruint_t arg3, ruint_t* err);
extern ruint_t dev_sys_4arg(ruint_t syscallnum, ruint_t arg1, ruint_t arg2, ruint_t arg3, ruint_t arg4);
extern ruint_t dev_sys_4arg_e(ruint_t syscallnum, ruint_t arg1, ruint_t arg2, ruint_t arg3, ruint_t arg4, ruint_t* err);
// 5arg has no error report natively
extern ruint_t dev_sys_5arg(ruint_t syscallnum, ruint_t arg1, ruint_t arg2, ruint_t arg3, ruint_t arg4, ruint_t arg5);
#ifdef __cplusplus
}
#endif
