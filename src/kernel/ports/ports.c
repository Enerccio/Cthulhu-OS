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
 * ports.c
 *  Created on: Dec 22, 2015
 *      Author: Peter Vanusanik
 *  Contents: port communication
 */

#include "ports.h"
#include "../memory/paging.h"

/** number maximum of available com ports, some might be unusable */
uint8_t   com_ports_count;
/** available com ports are listed here */
uint32_t  com_ports_address[4];
/** number maximum of available lpt ports, some might be unusable */
uint8_t   lpt_ports_count;
/** available lpt ports are listed here */
uint32_t  lpt_ports_address[3];
/** VGA memory address */
volatile uint16_t* video_memory;
/** EBDA memory address */
void*     ebda;

#define BDA_IO_COM ((uint16_t*)0x0400)
#define BDA_IO_LPT ((uint16_t*)0x0408)
#define BDA_EBDA ((uint32_t*)0x040E)

/**
 * Initializes serial port identified by pa.
 */
void init_serial_port(uint32_t pa) {
    if (pa == 0)
        return;

    outb(IOP_INTERRUPT_EN_REG(pa), 0x00);   // disable all interrupts
    outb(IOP_LINE_CONT_REG(pa), 0x80);      // Enable DLAB (set baud rate divisor)
    outb(IOP_DATA_BAUDR_LS_DIV(pa), 0x03);  // Set divisor to 3 (lo byte) 38400 baud
    outb(IOP_DATA_BAUDR_MS_DIV(pa), 0x00);  //                  (hi byte)
    outb(IOP_DLAB_REGISTER(pa), 0x03);      // 8 bits, no parity, one stop bit
    outb(IOP_INTERR_ID_FIFO_REG(pa), 0xC7); // Enable FIFO, clear them, with 14-byte threshold
    outb(IOP_MODEM_CONT_REG(pa), 0x0B);     // IRQs enabled, RTS/DSR set
}

/**
 * Initializes serial and lpt ports to be used by kernel.
 *
 * TODO: Add lpt initialization
 */
void initialize_ports() {
    ebda = (void*)physical_to_virtual((((uintptr_t)*BDA_EBDA)>>4));
    video_memory = (uint16_t*)physical_to_virtual((uint64_t)(0xB8000));

    com_ports_count = 4;
    memset(com_ports_address, 0, sizeof(uint32_t)*com_ports_count);
    for (uint8_t i=0; i<com_ports_count; i++) {
        com_ports_address[i] = *(uint16_t*)physical_to_virtual((uintptr_t)(BDA_IO_COM + i));
        init_serial_port(com_ports_address[i]);
    }

    lpt_ports_count = 3;
    memset(lpt_ports_address, 0, sizeof(uint32_t)*lpt_ports_count);
    for (uint8_t i=0; i<lpt_ports_count; i++) {
        lpt_ports_address[i] = *(uint16_t*)physical_to_virtual((uintptr_t)(BDA_IO_LPT + i));
    }
}

/**
 * Returns whether com line is empty or not for supplied port.
 */
bool com_empty_line(uint16_t port) {
    return inb(port+5) & 0x20;
}

/**
 * Writes data to com port.
 *
 * Determines address for com port, then waits until it is empty
 * and when it is, it sends out data.
 */
void write_byte_com(uint8_t com, uint8_t data) {
    uint16_t com_port = (uint16_t)com_ports_address[com];
    if (com_port == 0)
        return;
    while (!com_empty_line(com_port))
        ;
    outb(com_port, data);
}
