/*
 * The MIT License (MIT)
 * Copyright (c) 2015 Peter Vanusanik <admin@en-circle.com>
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights to
 * use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of
 * the Software, and to permit persons to whom the Software is furnished to do so,
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
 * ports.h
 *  Created on: Dec 22, 2015
 *      Author: Peter Vanusanik
 *  Contents: ports communication
 */

#pragma once

#include "../commons.h"
#include <stdint.h>
#include <string.h>

extern uint8_t  com_ports_count;
extern uint32_t com_ports_address[];
extern uint8_t  lpt_ports_count;
extern uint32_t lpt_ports_address[];
extern void*    ebda;

#define COM  0
#define COM1 (COM+0)
#define COM2 (COM+1)
#define COM3 (COM+2)
#define COM4 (COM+3)

#define IOP_DATA_REGISTER(addr)      (uint16_t)(addr+0)
#define IOP_INTERRUPT_EN_REG(addr)   (uint16_t)(addr+1)
#define IOP_DATA_BAUDR_LS_DIV(addr)  (uint16_t)(addr+0)
#define IOP_DATA_BAUDR_MS_DIV(addr)  (uint16_t)(addr+1)
#define IOP_INTERR_ID_FIFO_REG(addr) (uint16_t)(addr+2)
#define IOP_LINE_CONT_REG(addr)      (uint16_t)(addr+3)
#define IOP_DLAB_REGISTER(addr)      (uint16_t)(addr+3)
#define IOP_MODEM_CONT_REG(addr)     (uint16_t)(addr+4)
#define IOP_LINE_STATUS_REG(addr)    (uint16_t)(addr+5)
#define IOP_MODEM_STATUS_REG(addr)   (uint16_t)(addr+6)
#define IOP_SCRATCH_REGISTER(addr)   (uint16_t)(addr+7)

/**
 * Initializes com and lpt ports.
 */
void initialize_ports();

/**
 * Writes byte to com port.
 *
 * com must be one of COM1-COM4.
 */
void write_byte_com(uint8_t com, uint8_t data);
