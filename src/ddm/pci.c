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

static char* description [256] = {
	"Device was built prior definition of the class code field",
	"Mass Storage Controller ",
	"Network Controller ",
	"Display Controller ",
	"Multimedia Controller",
	"Memory Controller",
	"Bridge Device",
	"Simple Communication Controllers",
	"Base System Peripherals",
	"Input Devices",
	"Docking Stations",
	"Processors",
	"Serial Bus Controllers",
	"Wireless Controllers",
	"Intelligent I/O Controllers",
	"Satellite Communication Controllers",
	"Encryption/Decryption Controllers",
	"Data Acquisition and Signal Processing Controllers",
	"reserved","reserved","reserved","reserved","reserved","reserved","reserved","reserved","reserved","reserved",
	"reserved","reserved","reserved","reserved","reserved","reserved","reserved","reserved","reserved","reserved",
	"reserved","reserved","reserved","reserved","reserved","reserved","reserved","reserved","reserved","reserved",
	"reserved","reserved","reserved","reserved","reserved","reserved","reserved","reserved","reserved","reserved",
	"reserved","reserved","reserved","reserved","reserved","reserved","reserved","reserved","reserved","reserved",
	"reserved","reserved","reserved","reserved","reserved","reserved","reserved","reserved","reserved","reserved",
	"reserved","reserved","reserved","reserved","reserved","reserved","reserved","reserved","reserved","reserved",
	"reserved","reserved","reserved","reserved","reserved","reserved","reserved","reserved","reserved","reserved",
	"reserved","reserved","reserved","reserved","reserved","reserved","reserved","reserved","reserved","reserved",
	"reserved","reserved","reserved","reserved","reserved","reserved","reserved","reserved","reserved","reserved",
	"reserved","reserved","reserved","reserved","reserved","reserved","reserved","reserved","reserved","reserved",
	"reserved","reserved","reserved","reserved","reserved","reserved","reserved","reserved","reserved","reserved",
	"reserved","reserved","reserved","reserved","reserved","reserved","reserved","reserved","reserved","reserved",
	"reserved","reserved","reserved","reserved","reserved","reserved","reserved","reserved","reserved","reserved",
	"reserved","reserved","reserved","reserved","reserved","reserved","reserved","reserved","reserved","reserved",
	"reserved","reserved","reserved","reserved","reserved","reserved","reserved","reserved","reserved","reserved",
	"reserved","reserved","reserved","reserved","reserved","reserved","reserved","reserved","reserved","reserved",
	"reserved","reserved","reserved","reserved","reserved","reserved","reserved","reserved","reserved","reserved",
	"reserved","reserved","reserved","reserved","reserved","reserved","reserved","reserved","reserved","reserved",
	"reserved","reserved","reserved","reserved","reserved","reserved","reserved","reserved","reserved","reserved",
	"reserved","reserved","reserved","reserved","reserved","reserved","reserved","reserved","reserved","reserved",
	"reserved","reserved","reserved","reserved","reserved","reserved","reserved","reserved","reserved","reserved",
	"reserved","reserved","reserved","reserved","reserved","reserved","reserved","reserved","reserved","reserved",
	"reserved","reserved","reserved","reserved","reserved","reserved","reserved",
	"Device does not fit any defined class."
};

static char** descriptions;

ARRAY_FUNCDEFS(pcie_info_ptr_t)

pci_bus_t* pci_express_info;
pcie_info_array* pcie_entries;

union busaddr {
	struct {
		uint64_t offset : 2;
		uint64_t reg : 6;
		uint64_t ereg : 4;
		uint64_t func : 3;
		uint64_t device : 5;
		uint64_t bus : 8;
	} addr;
	uint64_t number;
};

uint64_t* config_qword(uintptr_t address,
		uint8_t busnum, uint8_t devicenum, uint8_t funcnum,
		uint8_t ereg, uint8_t reg, uint8_t offset) {
	union busaddr ba;
	ba.number = 0;
	ba.addr.bus = busnum;
	ba.addr.device = devicenum;
	ba.addr.ereg = ereg;
	ba.addr.func = funcnum;
	ba.addr.reg = reg;
	ba.addr.offset = offset;
	return ((uint64_t*)(ba.number + address));
}

uint32_t* config_dword(uintptr_t address,
		uint8_t busnum, uint8_t devicenum, uint8_t funcnum,
		uint8_t ereg, uint8_t reg, uint8_t offset) {
	union busaddr ba;
	ba.number = 0;
	ba.addr.bus = busnum;
	ba.addr.device = devicenum;
	ba.addr.ereg = ereg;
	ba.addr.func = funcnum;
	ba.addr.reg = reg;
	ba.addr.offset = offset;
	return ((uint32_t*)(ba.number + address));
}

uint16_t* config_word(uintptr_t address,
		uint8_t busnum, uint8_t devicenum, uint8_t funcnum,
		uint8_t ereg, uint8_t reg, uint8_t offset) {
	union busaddr ba;
	ba.number = 0;
	ba.addr.bus = busnum;
	ba.addr.device = devicenum;
	ba.addr.ereg = ereg;
	ba.addr.func = funcnum;
	ba.addr.reg = reg;
	ba.addr.offset = offset;
	return ((uint16_t*)(ba.number + address));
}

uint8_t* config_byte(uintptr_t address,
		uint8_t busnum, uint8_t devicenum, uint8_t funcnum,
		uint8_t ereg, uint8_t reg, uint8_t offset) {
	union busaddr ba;
	ba.number = 0;
	ba.addr.bus = busnum;
	ba.addr.device = devicenum;
	ba.addr.ereg = ereg;
	ba.addr.func = funcnum;
	ba.addr.reg = reg;
	ba.addr.offset = offset;
	return ((uint8_t*)(ba.number + address));
}

void config_write_byte(uintptr_t address,
		uint8_t busnum, uint8_t devicenum, uint8_t funcnum,
		uint8_t ereg, uint8_t reg, uint8_t offset, uint8_t data) {
	union busaddr ba;
	ba.number = 0;
	ba.addr.bus = busnum;
	ba.addr.device = devicenum;
	ba.addr.ereg = ereg;
	ba.addr.func = funcnum;
	ba.addr.reg = reg;
	ba.addr.offset = offset;
	*((uint8_t*)(ba.number + address)) = data;
}

void load_pcie_entry_info(pci_bus_t* bi) {
	size_t numentries = bi->end_pci_busnum - bi->start_pci_busnum;
	void* pci_express_infoaddr = self_map_physical(bi->base_address, numentries*4096*8*32);
	if (pci_express_infoaddr == NULL)
		return;
	for (size_t i=0; i<numentries; i++) {
		for (int j=0; j<32; j++) {
			uint16_t vendor_id = *config_word((uintptr_t)pci_express_infoaddr, i, j, 0, 0, 0x0, 2);
			if (vendor_id == 0xFFFF)
				continue;

			pcie_info_t* info = malloc(sizeof(pcie_info_t));
			if (info == NULL) {
				// TODO: add kernel shutdown
				return;
			}
			memset(info, 0, sizeof(pcie_info_t));

			info->base_address = (uintptr_t)pci_express_infoaddr;
			info->bus = i;
			info->device = j;
			info->vendor_id = vendor_id;
			info->device_id = *config_word(info->base_address, info->bus, info->device, 0, 0, 0x0, 0);
			info->status = config_word(info->base_address, info->bus, info->device, 0, 0, 1, 0);
			info->command = config_word(info->base_address, info->bus, info->device, 0, 0, 1, 2);
			info->class = *config_byte(info->base_address, info->bus, info->device, 0, 0, 2, 3);
			info->subclass = *config_byte(info->base_address, info->bus, info->device, 0, 0, 2, 2);
			info->prog_if = *config_byte(info->base_address, info->bus, info->device, 0, 0, 2, 1);
			info->rev_id = *config_byte(info->base_address, info->bus, info->device, 0, 0, 2, 0);
			info->bist = config_byte(info->base_address, info->bus, info->device, 0, 0, 3, 3);
			info->htype = *config_byte(info->base_address, info->bus, info->device, 0, 0, 3, 2);
			info->lat_timer = *config_byte(info->base_address, info->bus, info->device, 0, 0, 3, 1);
			info->cache = *config_byte(info->base_address, info->bus, info->device, 0, 0, 3, 0);

			info->cdescription = description[info->class];
			info->ddescription = descriptions[(info->class*128*128)+(info->subclass*128)+info->prog_if];


			size_t cac = array_get_size(pcie_info_ptr_t, pcie_entries);
			if (array_push_data(pcie_info_ptr_t, pcie_entries, info) == cac) {
				// TODO: add kernel shutdown
				return;
			}
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

	descriptions = malloc(sizeof(char*)*256*256*256);
	if (descriptions == NULL) {
		// TODO: add kernel shutdown
		return;
	}

	for (int cc=0; cc<128; cc++)
		for (int sc=0; sc<128; sc++)
			for (int p=0; p<128; p++) {
				if (cc == 0 && sc == 0 && p == 0)
					descriptions[(cc*128*128)+(sc*128)+p] = "Any device except for VGA-Compatible devices";
				else if (cc == 0 && sc == 0 && p == 1)
					descriptions[(cc*128*128)+(sc*128)+p] = "VGA-Compatible Device";

				else if (cc == 1 && sc == 0 && p == 0)
					descriptions[(cc*128*128)+(sc*128)+p] = "SCSI Bus Controller";
				else if (cc == 1 && sc == 1)
					descriptions[(cc*128*128)+(sc*128)+p] = "IDE Controller";
				else if (cc == 1 && sc == 2 && p == 0)
					descriptions[(cc*128*128)+(sc*128)+p] = "Floppy Disk Controller";
				else if (cc == 1 && sc == 3 && p == 0)
					descriptions[(cc*128*128)+(sc*128)+p] = "IPI Bus Controller";
				else if (cc == 1 && sc == 4 && p == 0)
					descriptions[(cc*128*128)+(sc*128)+p] = "RAID Controller";
				else if (cc == 1 && sc == 5 && p == 0x20)
					descriptions[(cc*128*128)+(sc*128)+p] = "ATA Controller (Single DMA)";
				else if (cc == 1 && sc == 5 && p == 0x30)
					descriptions[(cc*128*128)+(sc*128)+p] = "ATA Controller (Chained DMA)";
				else if (cc == 1 && sc == 6 && p == 0)
					descriptions[(cc*128*128)+(sc*128)+p] = "Serial ATA (Vendor Specific Interface)";
				else if (cc == 1 && sc == 6 && p == 1)
					descriptions[(cc*128*128)+(sc*128)+p] = "Serial ATA (AHCI 1.0)";
				else if (cc == 1 && sc == 7 && p == 0)
					descriptions[(cc*128*128)+(sc*128)+p] = "Serial Attached SCSI (SAS)";
				else if (cc == 1 && sc == 0x80 && p == 0)
					descriptions[(cc*128*128)+(sc*128)+p] = "Other Mass Storage Controller ";

				else if (cc == 2 && sc == 0 && p == 0)
					descriptions[(cc*128*128)+(sc*128)+p] = "Ethernet Controller";
				else if (cc == 2 && sc == 1 && p == 0)
					descriptions[(cc*128*128)+(sc*128)+p] = "Token Ring Controller";
				else if (cc == 2 && sc == 2 && p == 0)
					descriptions[(cc*128*128)+(sc*128)+p] = "FDDI Controller";
				else if (cc == 2 && sc == 3 && p == 0)
					descriptions[(cc*128*128)+(sc*128)+p] = "ATM Controller";
				else if (cc == 2 && sc == 4 && p == 0)
					descriptions[(cc*128*128)+(sc*128)+p] = "ISDN Controller";
				else if (cc == 2 && sc == 5 && p == 0)
					descriptions[(cc*128*128)+(sc*128)+p] = "WorldFip Controller";
				else if (cc == 2 && sc == 6)
					descriptions[(cc*128*128)+(sc*128)+p] = "PICMG 2.14 Multi Computing";
				else if (cc == 2 && sc == 0x80 && p == 0)
					descriptions[(cc*128*128)+(sc*128)+p] = "Other Network Controller";

				else if (cc == 3 && sc == 0 && p == 0)
					descriptions[(cc*128*128)+(sc*128)+p] = "VGA-Compatible Controller";
				else if (cc == 3 && sc == 0 && p == 1)
					descriptions[(cc*128*128)+(sc*128)+p] = "8512-Compatible Controller";
				else if (cc == 3 && sc == 1 && p == 0)
					descriptions[(cc*128*128)+(sc*128)+p] = "XGA Controller";
				else if (cc == 3 && sc == 2 && p == 0)
					descriptions[(cc*128*128)+(sc*128)+p] = "3D Controller (Not VGA-Compatible)";
				else if (cc == 3 && sc == 0x80 && p == 0)
					descriptions[(cc*128*128)+(sc*128)+p] = "Other Display Controller";

				else if (cc == 4 && sc == 0 && p == 0)
					descriptions[(cc*128*128)+(sc*128)+p] = "Video Device";
				else if (cc == 4 && sc == 1 && p == 0)
					descriptions[(cc*128*128)+(sc*128)+p] = "Audio Device";
				else if (cc == 4 && sc == 2 && p == 0)
					descriptions[(cc*128*128)+(sc*128)+p] = "Computer Telephony Device";
				else if (cc == 4 && sc == 0x80 && p == 0)
					descriptions[(cc*128*128)+(sc*128)+p] = "Other Multimedia Device";

				else if (cc == 5 && sc == 0 && p == 0)
					descriptions[(cc*128*128)+(sc*128)+p] = "RAM Controller";
				else if (cc == 5 && sc == 1 && p == 0)
					descriptions[(cc*128*128)+(sc*128)+p] = "Flash Controller";
				else if (cc == 5 && sc == 0x80 && p == 0)
					descriptions[(cc*128*128)+(sc*128)+p] = "Other Memory Controller";

				else if (cc == 6 && sc == 0 && p == 0)
					descriptions[(cc*128*128)+(sc*128)+p] = "Host Bridge";
				else if (cc == 6 && sc == 1 && p == 0)
					descriptions[(cc*128*128)+(sc*128)+p] = "ISA Bridge";
				else if (cc == 6 && sc == 2 && p == 0)
					descriptions[(cc*128*128)+(sc*128)+p] = "EISA Bridge";
				else if (cc == 6 && sc == 3 && p == 0)
					descriptions[(cc*128*128)+(sc*128)+p] = "MCA Bridge";
				else if (cc == 6 && sc == 4 && p == 0)
					descriptions[(cc*128*128)+(sc*128)+p] = "PCI-to-PCI Bridge";
				else if (cc == 6 && sc == 4 && p == 1)
					descriptions[(cc*128*128)+(sc*128)+p] = "PCI-to-PCI Bridge (Subtractive Decode)";
				else if (cc == 6 && sc == 5 && p == 0)
					descriptions[(cc*128*128)+(sc*128)+p] = "PCMCIA Bridge";
				else if (cc == 6 && sc == 6 && p == 0)
					descriptions[(cc*128*128)+(sc*128)+p] = "NuBus Bridge";
				else if (cc == 6 && sc == 7 && p == 0)
					descriptions[(cc*128*128)+(sc*128)+p] = "CardBus Bridge";
				else if (cc == 6 && sc == 8)
					descriptions[(cc*128*128)+(sc*128)+p] = "RACEway Bridge";
				else if (cc == 6 && sc == 9 && p == 0x40)
					descriptions[(cc*128*128)+(sc*128)+p] = "PCI-to-PCI Bridge (Semi-Transparent, Primary)";
				else if (cc == 6 && sc == 9 && p == 0x80)
					descriptions[(cc*128*128)+(sc*128)+p] = "PCI-to-PCI Bridge (Semi-Transparent, Secondary)";
				else if (cc == 6 && sc == 0xA && p == 0)
					descriptions[(cc*128*128)+(sc*128)+p] = "InfiniBrand-to-PCI Host Bridge";
				else if (cc == 6 && sc == 0x80 && p == 0)
					descriptions[(cc*128*128)+(sc*128)+p] = "Other Bridge Device";

				else if (cc == 7 && sc == 0 && p == 0)
					descriptions[(cc*128*128)+(sc*128)+p] = "Generic XT-Compatible Serial Controller";
				else if (cc == 7 && sc == 0 && p == 1)
					descriptions[(cc*128*128)+(sc*128)+p] = "16450-Compatible Serial Controller";
				else if (cc == 7 && sc == 0 && p == 2)
					descriptions[(cc*128*128)+(sc*128)+p] = "16550-Compatible Serial Controller";
				else if (cc == 7 && sc == 0 && p == 3)
					descriptions[(cc*128*128)+(sc*128)+p] = "16650-Compatible Serial Controller";
				else if (cc == 7 && sc == 0 && p == 4)
					descriptions[(cc*128*128)+(sc*128)+p] = "16750-Compatible Serial Controller";
				else if (cc == 7 && sc == 0 && p == 5)
					descriptions[(cc*128*128)+(sc*128)+p] = "16850-Compatible Serial Controller";
				else if (cc == 7 && sc == 0 && p == 6)
					descriptions[(cc*128*128)+(sc*128)+p] = "16950-Compatible Serial Controller";
				else if (cc == 7 && sc == 1 && p == 0)
					descriptions[(cc*128*128)+(sc*128)+p] = "Parallel Port";
				else if (cc == 7 && sc == 1 && p == 1)
					descriptions[(cc*128*128)+(sc*128)+p] = "Bi-Directional Parallel Port";
				else if (cc == 7 && sc == 1 && p == 2)
					descriptions[(cc*128*128)+(sc*128)+p] = "ECP 1.X Compliant Parallel Port";
				else if (cc == 7 && sc == 1 && p == 3)
					descriptions[(cc*128*128)+(sc*128)+p] = "IEEE 1284 Controller";
				else if (cc == 7 && sc == 1 && p == 0xFE)
					descriptions[(cc*128*128)+(sc*128)+p] = "IEEE 1284 Target Device";
				else if (cc == 7 && sc == 2 && p == 0)
					descriptions[(cc*128*128)+(sc*128)+p] = "Multiport Serial Controller";
				else if (cc == 7 && sc == 3 && p == 0)
					descriptions[(cc*128*128)+(sc*128)+p] = "Generic Modem";
				else if (cc == 7 && sc == 3 && p == 1)
					descriptions[(cc*128*128)+(sc*128)+p] = "Hayes Compatible Modem (16450-Compatible Interface)";
				else if (cc == 7 && sc == 3 && p == 2)
					descriptions[(cc*128*128)+(sc*128)+p] = "Hayes Compatible Modem (16550-Compatible Interface)";
				else if (cc == 7 && sc == 3 && p == 3)
					descriptions[(cc*128*128)+(sc*128)+p] = "Hayes Compatible Modem (16650-Compatible Interface)";
				else if (cc == 7 && sc == 3 && p == 4)
					descriptions[(cc*128*128)+(sc*128)+p] = "Hayes Compatible Modem (16750-Compatible Interface)";
				else if (cc == 7 && sc == 4 && p == 0)
					descriptions[(cc*128*128)+(sc*128)+p] = "IEEE 488.1/2 (GPIB) Controller";
				else if (cc == 7 && sc == 5 && p == 0)
					descriptions[(cc*128*128)+(sc*128)+p] = "Smart Card";
				else if (cc == 7 && sc == 0x80 && p == 0)
					descriptions[(cc*128*128)+(sc*128)+p] = "Other Communications Device";

				else if (cc == 8 && sc == 0 && p == 0)
					descriptions[(cc*128*128)+(sc*128)+p] = "Generic 8259 PIC";
				else if (cc == 8 && sc == 0 && p == 1)
					descriptions[(cc*128*128)+(sc*128)+p] = "ISA PIC";
				else if (cc == 8 && sc == 0 && p == 2)
					descriptions[(cc*128*128)+(sc*128)+p] = "EISA PIC";
				else if (cc == 8 && sc == 0 && p == 0x10)
					descriptions[(cc*128*128)+(sc*128)+p] = "I/O APIC Interrupt Controller";
				else if (cc == 8 && sc == 0 && p == 0x20)
					descriptions[(cc*128*128)+(sc*128)+p] = "I/O(x) APIC Interrupt Controller";
				else if (cc == 8 && sc == 1 && p == 0)
					descriptions[(cc*128*128)+(sc*128)+p] = "Generic 8237 DMA Controller";
				else if (cc == 8 && sc == 1 && p == 1)
					descriptions[(cc*128*128)+(sc*128)+p] = "ISA DMA Controller";
				else if (cc == 8 && sc == 1 && p == 2)
					descriptions[(cc*128*128)+(sc*128)+p] = "EISA DMA Controller";
				else if (cc == 8 && sc == 2 && p == 0)
					descriptions[(cc*128*128)+(sc*128)+p] = "Generic 8254 System Timer";
				else if (cc == 8 && sc == 2 && p == 1)
					descriptions[(cc*128*128)+(sc*128)+p] = "ISA System Timer";
				else if (cc == 8 && sc == 2 && p == 2)
					descriptions[(cc*128*128)+(sc*128)+p] = "EISA System Timer";
				else if (cc == 8 && sc == 3 && p == 0)
					descriptions[(cc*128*128)+(sc*128)+p] = "Generic RTC Controller";
				else if (cc == 8 && sc == 3 && p == 1)
					descriptions[(cc*128*128)+(sc*128)+p] = "ISA RTC Controller";
				else if (cc == 8 && sc == 4 && p == 0)
					descriptions[(cc*128*128)+(sc*128)+p] = "Generic PCI Hot-Plug Controller";
				else if (cc == 8 && sc == 0x80 && p == 0)
					descriptions[(cc*128*128)+(sc*128)+p] = "Other System Peripheral";

				else if (cc == 9 && sc == 0x0 && p == 0)
					descriptions[(cc*128*128)+(sc*128)+p] = "Keyboard Controller";
				else if (cc == 9 && sc == 0x1 && p == 0)
					descriptions[(cc*128*128)+(sc*128)+p] = "Digitizer";
				else if (cc == 9 && sc == 0x2 && p == 0)
					descriptions[(cc*128*128)+(sc*128)+p] = "Mouse Controller";
				else if (cc == 9 && sc == 0x3 && p == 0)
					descriptions[(cc*128*128)+(sc*128)+p] = "Scanner Controller";
				else if (cc == 9 && sc == 0x4 && p == 0)
					descriptions[(cc*128*128)+(sc*128)+p] = "Gameport Controller (Generic)";
				else if (cc == 9 && sc == 0x4 && p == 0x10)
					descriptions[(cc*128*128)+(sc*128)+p] = "Gameport Contrlller (Legacy)";
				else if (cc == 9 && sc == 0x80 && p == 0)
					descriptions[(cc*128*128)+(sc*128)+p] = "Other Input Controller";

				else if (cc == 0xA && sc == 0x0 && p == 0)
					descriptions[(cc*128*128)+(sc*128)+p] = "Generic Docking Station";
				else if (cc == 0xA && sc == 0x80 && p == 0)
					descriptions[(cc*128*128)+(sc*128)+p] = "Other Docking Station";

				else if (cc == 0xB && sc == 0x0 && p == 0)
					descriptions[(cc*128*128)+(sc*128)+p] = "386 Processor";
				else if (cc == 0xB && sc == 0x1 && p == 0)
					descriptions[(cc*128*128)+(sc*128)+p] = "486 Processor";
				else if (cc == 0xB && sc == 0x2 && p == 0)
					descriptions[(cc*128*128)+(sc*128)+p] = "Pentium Processor";
				else if (cc == 0xB && sc == 0x10 && p == 0)
					descriptions[(cc*128*128)+(sc*128)+p] = "Alpha Processor";
				else if (cc == 0xB && sc == 0x20 && p == 0)
					descriptions[(cc*128*128)+(sc*128)+p] = "PowerPC Processor";
				else if (cc == 0xB && sc == 0x30 && p == 0)
					descriptions[(cc*128*128)+(sc*128)+p] = "MIPS Processor";
				else if (cc == 0xB && sc == 0x40 && p == 0)
					descriptions[(cc*128*128)+(sc*128)+p] = "Co-Processor";

				else if (cc == 0xC && sc == 0x0 && p == 0)
					descriptions[(cc*128*128)+(sc*128)+p] = "IEEE 1394 Controller (FireWire)";
				else if (cc == 0xC && sc == 0x0 && p == 0x10)
					descriptions[(cc*128*128)+(sc*128)+p] = "IEEE 1394 Controller (1394 OpenHCI Spec)";
				else if (cc == 0xC && sc == 0x1 && p == 0)
					descriptions[(cc*128*128)+(sc*128)+p] = "ACCESS.bus";
				else if (cc == 0xC && sc == 0x2 && p == 0)
					descriptions[(cc*128*128)+(sc*128)+p] = "SSA";
				else if (cc == 0xC && sc == 0x3 && p == 0)
					descriptions[(cc*128*128)+(sc*128)+p] = "USB (Universal Host Controller Spec)";
				else if (cc == 0xC && sc == 0x3 && p == 0x10)
					descriptions[(cc*128*128)+(sc*128)+p] = "USB (Open Host Controller Spec)";
				else if (cc == 0xC && sc == 0x3 && p == 0x20)
					descriptions[(cc*128*128)+(sc*128)+p] = "USB2 Host Controller (Intel Enhanced Host Controller Interface)";
				else if (cc == 0xC && sc == 0x3 && p == 0x30)
					descriptions[(cc*128*128)+(sc*128)+p] = "USB3 XHCI Controller";
				else if (cc == 0xC && sc == 0x3 && p == 0x80)
					descriptions[(cc*128*128)+(sc*128)+p] = "Unspecified USB Controller";
				else if (cc == 0xC && sc == 0xFE)
					descriptions[(cc*128*128)+(sc*128)+p] = "USB (Not Host Controller)";
				else if (cc == 0xC && sc == 0x4 && p == 0)
					descriptions[(cc*128*128)+(sc*128)+p] = "Fibre Channel";
				else if (cc == 0xC && sc == 0x5 && p == 0)
					descriptions[(cc*128*128)+(sc*128)+p] = "SMBus";
				else if (cc == 0xC && sc == 0x6 && p == 0)
					descriptions[(cc*128*128)+(sc*128)+p] = "InfiniBand";
				else if (cc == 0xC && sc == 0x7 && p == 0)
					descriptions[(cc*128*128)+(sc*128)+p] = "IPMI SMIC Interface";
				else if (cc == 0xC && sc == 0x7 && p == 1)
					descriptions[(cc*128*128)+(sc*128)+p] = "IPMI Kybd Controller Style Interface";
				else if (cc == 0xC && sc == 0x7 && p == 2)
					descriptions[(cc*128*128)+(sc*128)+p] = "IPMI Block Transfer Interface";
				else if (cc == 0xC && sc == 0x8 && p == 0)
					descriptions[(cc*128*128)+(sc*128)+p] = "SERCOS Interface Standard (IEC 61491)";
				else if (cc == 0xC && sc == 0x9 && p == 0)
					descriptions[(cc*128*128)+(sc*128)+p] = "CANbus";

				else if (cc == 0xD && sc == 0x0 && p == 0)
					descriptions[(cc*128*128)+(sc*128)+p] = "iRDA Compatible Controller";
				else if (cc == 0xD && sc == 0x1 && p == 0)
					descriptions[(cc*128*128)+(sc*128)+p] = "Consumer IR Controller";
				else if (cc == 0xD && sc == 0x10 && p == 0)
					descriptions[(cc*128*128)+(sc*128)+p] = "RF Controller";
				else if (cc == 0xD && sc == 0x11 && p == 0)
					descriptions[(cc*128*128)+(sc*128)+p] = "Bluetooth Controller";
				else if (cc == 0xD && sc == 0x12 && p == 0)
					descriptions[(cc*128*128)+(sc*128)+p] = "Broadband Controller";
				else if (cc == 0xD && sc == 0x20 && p == 0)
					descriptions[(cc*128*128)+(sc*128)+p] = "Ethernet Controller (802.11a)";
				else if (cc == 0xD && sc == 0x21 && p == 0)
					descriptions[(cc*128*128)+(sc*128)+p] = "Ethernet Controller (802.11b)";
				else if (cc == 0xD && sc == 0x80 && p == 0)
					descriptions[(cc*128*128)+(sc*128)+p] = "Other Wireless Controller";

				else if (cc == 0xE && sc == 0x0 && p == 0)
					descriptions[(cc*128*128)+(sc*128)+p] = "I20 Architecture";
				else if (cc == 0xE && sc == 0x0 && p > 0)
					descriptions[(cc*128*128)+(sc*128)+p] = "Message FIFO";

				else if (cc == 0xF && sc == 0x1 && p == 0)
					descriptions[(cc*128*128)+(sc*128)+p] = "TV Controller";
				else if (cc == 0xF && sc == 0x2 && p == 0)
					descriptions[(cc*128*128)+(sc*128)+p] = "Audio Controller";
				else if (cc == 0xF && sc == 0x3 && p == 0)
					descriptions[(cc*128*128)+(sc*128)+p] = "Voice Controller";
				else if (cc == 0xF && sc == 0x4 && p == 0)
					descriptions[(cc*128*128)+(sc*128)+p] = "Data Controller";

				else if (cc == 0x10 && sc == 0x0 && p == 0)
					descriptions[(cc*128*128)+(sc*128)+p] = "Network and Computing Encryption/Decryption";
				else if (cc == 0x10 && sc == 0x10 && p == 0)
					descriptions[(cc*128*128)+(sc*128)+p] = "Entertainment Encryption/Decryption";
				else if (cc == 0x10 && sc == 0x80 && p == 0)
					descriptions[(cc*128*128)+(sc*128)+p] = "Other Encryption/Decryption";

				else if (cc == 0x11 && sc == 0x0 && p == 0)
					descriptions[(cc*128*128)+(sc*128)+p] = "DPIO Modules";
				else if (cc == 0x11 && sc == 0x1 && p == 0)
					descriptions[(cc*128*128)+(sc*128)+p] = "Performance Counters";
				else if (cc == 0x11 && sc == 0x10 && p == 0)
					descriptions[(cc*128*128)+(sc*128)+p] = "Communications Synchronization Plus Time and Frequency Test/Measurement";
				else if (cc == 0x11 && sc == 0x20 && p == 0)
					descriptions[(cc*128*128)+(sc*128)+p] = "Management Card";
				else if (cc == 0x11 && sc == 0x80 && p == 0)
					descriptions[(cc*128*128)+(sc*128)+p] = "Other Data Acquisition/Signal Processing Controller";

				else
					descriptions[(cc*128*128)+(sc*128)+p] = "not specified";
			}

	load_pcie_info();
}
