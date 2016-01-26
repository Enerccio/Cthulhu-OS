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
 * pci.c
 *  Created on: Jan 26, 2016
 *      Author: Peter Vanusanik
 *  Contents: 
 */

#include "pci.h"

ARRAY_FUNCDEFS(pcie_info_ptr_t)

pci_bus_t* pci_express_info;
pcie_info_array* pcie_entries;

void load_pcie_entry_info(pci_bus_t* bi) {
	size_t numentries = bi->end_pci_busnum - bi->start_pci_busnum;
	void* pci_express_infoaddr = self_map_physical(bi->base_address, numentries*4096*8);
	if (pci_express_infoaddr == NULL)
		return;
	for (size_t i=0; i<numentries; i++) {
		pcie_info_t* info = malloc(sizeof(pcie_info_t));
		if (info == NULL) {
			// TODO: add kernel shutdown
			return;
		}
		memset(info, 0, sizeof(pcie_info_t));
		info->probeaddr = (void*)(((char*)pci_express_infoaddr) + (8 * 4096 * i));
		info->vendor_id = *((uint16_t*)info->probeaddr);
		info->device_id = *(((uint16_t*)info->probeaddr)+1);

		if (info->device_id == 0xffff && info->vendor_id == 0xffff) {
			free(info);
			continue;
		}

		size_t cac = array_get_size(pcie_info_ptr_t, pcie_entries);
		if (array_push_data(pcie_info_ptr_t, pcie_entries, info) == cac) {
			// TODO: add kernel shutdown
			return;
		}
	}
}

void load_pcie_info() {
	int64_t pci_busc = get_pci_bus_count();
	if (pci_busc <= 0) {
		return; // no pcie
	}

	pci_express_info = malloc(sizeof(pci_bus_t) * pci_busc);
	if (pci_express_info == NULL) {
		// TODO: add kernel shutdown
		return;
	}
	get_pci_info(pci_express_info);
	for (int64_t i=0; i<pci_busc; i++) {
		load_pcie_entry_info(&pci_express_info[i]);
	}
}

void load_pci_info() {
	pcie_entries = create_array(pcie_info_ptr_t);
	if (pcie_entries == NULL) {
		// TODO: add kernel shutdown
		return;
	}

	load_pcie_info();
}
