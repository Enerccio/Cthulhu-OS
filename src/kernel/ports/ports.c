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

uint8_t   com_ports_count;
uint32_t  com_ports_address[4];
uint8_t   lpt_ports_count;
uint32_t  lpt_ports_address[3];
uint16_t* video_memory; // (uint16_t*) 0xB8000;
void*     ebda;

#define VIDEO_MEM_OFFSET (0xB8000-0xA0000)
#define BDA_IO_COM ((uint32_t*)0x0400)
#define BDA_IO_LPT ((uint32_t*)0x0408)
#define BDA_EBDA ((uint32_t*)0x040E)

void init_serial_port(uint32_t pa){
	if (pa == 0)
		return;

	outb(IOP_INTERRUPT_EN_REG(pa), 0x00);   // disable all interrupts
	outb(IOP_LINE_CONT_REG(pa), 0x80); 		// Enable DLAB (set baud rate divisor)
	outb(IOP_DATA_BAUDR_LS_DIV(pa), 0x03);  // Set divisor to 3 (lo byte) 38400 baud
	outb(IOP_DATA_BAUDR_MS_DIV(pa), 0x00);  //                  (hi byte)
	outb(IOP_DLAB_REGISTER(pa), 0x03);      // 8 bits, no parity, one stop bit
	outb(IOP_INTERR_ID_FIFO_REG(pa), 0xC7); // Enable FIFO, clear them, with 14-byte threshold
	outb(IOP_MODEM_CONT_REG(pa), 0x0B);     // IRQs enabled, RTS/DSR set
}

void initialize_ports(){
	ebda = (void*) (((uintptr_t)*BDA_EBDA)>>4);
	video_memory = (uint16_t*)(0xB8000);

	com_ports_count = 4;
	memset(com_ports_address, 0, sizeof(uint32_t)*com_ports_count);
	for (uint8_t i=0; i<com_ports_count; i++){
		com_ports_address[i] = *(BDA_IO_COM + i);
		init_serial_port(com_ports_address[i]);
	}

	lpt_ports_count = 3;
	memset(lpt_ports_address, 0, sizeof(uint32_t)*lpt_ports_count);
	for (uint8_t i=0; i<lpt_ports_count; i++){
		lpt_ports_address[i] = *(BDA_IO_LPT + i);
	}
}

bool com_empty_line(uint16_t port){
	return inb(port+5) & 0x20;
}

void write_byte_com(uint8_t com, uint8_t data){
	uint16_t com_port = (uint16_t)com_ports_address[com];
	if (com_port == 0)
		return;
	while (!com_empty_line(com_port))
		;
	outb(com_port, data);
}
