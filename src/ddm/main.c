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
 * main.c
 *  Created on: Jan 2, 2016
 *      Author: Peter Vanusanik
 *  Contents: 
 */

#include <ny/nyarlathotep.h>
#include <cthulhu/messages.h>
#include <cthulhu/process.h>
#include "pci.h"
#include "drivers.h"

void load_disk_drive(char* driver_path) {
	void* reply = malloc(MESSAGE_BODY_SIZE);
	if (reply == NULL) {
		// halt kernel
	}

	message_t* message = get_free_message();
	memset(message, 0, sizeof(message_t));
	message->header.flags.no_target = true;
	message->header.magic = MESSAGE_MAGIC;

	cp_stage1* cp1 = (cp_stage1*)message->data;
	cp1->parent = true;
	cp1->priority = -1;

	send_message(message);
	receive_message(reply);
}

void load_disk_drives() {
	for (uint32_t i=0; i<array_get_size(pcie_info_ptr_t, pcie_entries); i++) {
		pcie_info_t* info = array_get_at(pcie_info_ptr_t, pcie_entries, i);
		if (info->class == 0x1) {
			// disk class
			char* path = table_get(string, string, drivers, (char*)info->ddescription);
			if (path != NULL) {
				load_disk_drive(path);
			}
		}
	}
}

int main() {
    register_as_service("::device-driver-manager");
    load_pci_info();
    load_from_initramfs("/conf/ddm/drivers.conf");

    load_disk_drives();

    while (1) ;
}
