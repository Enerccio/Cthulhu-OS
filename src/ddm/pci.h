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
 * pci.h
 *  Created on: Jan 26, 2016
 *      Author: Peter Vanusanik
 *  Contents: 
 */

#pragma once

#include <stdbool.h>

#include <ny/nyarlathotep.h>
#include <datastruct/array.h>
#include <cthulhu/mmio.h>

typedef struct {
	uintptr_t   base_address;
	void       *exact_address;
	uint8_t     bus, device, function;
	uint16_t    device_id, vendor_id;
	void       *status, *command;
	uint8_t     class, subclass;
	uint8_t     prog_if, rev_id;
	void       *bist;
	uint8_t     htype;
	uint8_t     lat_timer, cache;

	const char *cdescription, *ddescription;
} pcie_info_t;

typedef pcie_info_t* pcie_info_ptr_t;
ARRAY_HEADER(pcie_info_ptr_t)
typedef ARRAY_TYPE(pcie_info_ptr_t) pcie_info_array;

void load_pci_info();

uint32_t* config_dword(uintptr_t address,
		uint8_t busnum, uint8_t devicenum, uint8_t funcnum,
		uint8_t ereg, uint8_t reg);

uint8_t config_read_byte(uintptr_t address,
		uint8_t busnum, uint8_t devicenum, uint8_t funcnum,
		uint8_t ereg, uint8_t reg, uint8_t offset);
uint16_t config_read_word(uintptr_t address,
		uint8_t busnum, uint8_t devicenum, uint8_t funcnum,
		uint8_t ereg, uint8_t reg, uint8_t offset);
uint32_t config_read_dword(uintptr_t address,
		uint8_t busnum, uint8_t devicenum, uint8_t funcnum,
		uint8_t ereg, uint8_t reg);
uint64_t config_read_qword(uintptr_t address,
		uint8_t busnum, uint8_t devicenum, uint8_t funcnum,
		uint8_t ereg, uint8_t reg);

extern pcie_info_array* pcie_entries;
