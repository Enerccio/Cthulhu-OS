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
 * scheduler.h
 *  Created on: Jan 1, 2016
 *      Author: Peter Vanusanik
 *  Contents: 
 */

#pragma once

#include "../commons.h"
#include "../interrupts/clock.h"
#include "../cpus/cpu_mgmt.h"
#include "process.h"

#include <ds/random.h>
#include <ds/array.h>

void attemp_to_run_scheduler(registers_t* r);
void schedule(registers_t* r);
void enschedule(thread_t* t, cpu_t* cpu);
void enschedule_best(thread_t* t);
void enschedule_to_self(thread_t* t);

void copy_registers(registers_t* r, thread_t* t);
void registers_copy(thread_t* t, registers_t* r);

void initialize_scheduler();

uint64_t new_mutex();
void unblock_mutex_waits(uint64_t mtxid);
void block_mutex_waits(uint64_t mtxid);
void block_wait_mutex(uint64_t mtxid, registers_t* registers);
