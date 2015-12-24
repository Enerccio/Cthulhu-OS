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
 * rsdt.c
 *  Created on: Dec 24, 2015
 *      Author: Peter Vanusanik
 *  Contents: 
 */

#include "acpi.h"

struct RSDPDescriptor* rsdp_descriptor;
RSDT*   rsdt;
XSDT*   xsdt;
FADT*   fadt;
uint8_t acpi_version;

extern void* ebda;

uintmax_t add_bytes(uint8_t* addr, size_t len) {
	uintmax_t v = 0;
	for (size_t p = 0; p < len ; p++)
		v += addr[p];
	return v;
}

bool acpisdt_checksum(ACPISDTHeader* tableHeader) {
	unsigned char sum = 0;

	for (unsigned int i = 0; i < tableHeader->Length; i++){
        sum += ((char *) tableHeader)[i];
    }

    return sum == 0;
}

bool valid_rsdt(RSDT* rsdt) {
	return acpisdt_checksum((ACPISDTHeader*)rsdt);
}

bool valid_xsdt(XSDT* xsdt) {
	return acpisdt_checksum((ACPISDTHeader*)xsdt);
}

void* findFACP(void* RootSDT) {
    RSDT* rsdt = (RSDT*) RootSDT;
    int entries = (rsdt->h.Length - sizeof(rsdt->h)) / 4;

    for (int i=0; i<entries; i++){
        ACPISDTHeader* h = (ACPISDTHeader*) physical_to_virtual((uint64_t)(&rsdt->PointerToOtherSDT)[i]);
        if (!strncmp(h->Signature, "FACP", 4))
            return (void*)h;
    }

    // No FACP found
    return NULL;
}

void* findFACP_XSDT(void* RootSDT) {
    XSDT* xsdt = (XSDT*) RootSDT;
    int entries = (xsdt->h.Length - sizeof(xsdt->h)) / 8;

    for (int i=0; i<entries; i++){
        ACPISDTHeader* h = (ACPISDTHeader*) physical_to_virtual((uint64_t)(&rsdt->PointerToOtherSDT)[i]);
        if (!strncmp(h->Signature, "FACP", 4))
            return (void*)h;
    }

    // No FACP found
    return NULL;
}

void* check_madt_type(ACPISDTHeader* h, enum acpi_madt_type type) {
	MADT_HEADER* madt = (MADT_HEADER*)h;
	ACPI_SUBTABLE_HEADER* ptr = (ACPI_SUBTABLE_HEADER*)physical_to_virtual((uint64_t)madt->address);
	if (ptr->type == type){
		return ptr;
	}
	return NULL;
}

void* find_madt(enum acpi_madt_type type, unsigned int ccount) {
	if (acpi_version == 1){
		if (rsdt == NULL)
			return NULL;
		int entries = (rsdt->h.Length - sizeof(rsdt->h)) / 4;
		for (int i=0; i<entries; i++){
			ACPISDTHeader* h = (ACPISDTHeader*) physical_to_virtual((uint64_t)(&rsdt->PointerToOtherSDT)[i]);
			if (!strncmp(h->Signature, "APIC", 4)){
				void* addr;
				if ((addr = check_madt_type(h, type))!=NULL){
					if (ccount == 0)
						return (void*)addr;
					else
						--ccount;
				}
			}
		}
	} else {
		if (xsdt == NULL)
			return NULL;
		int entries = (xsdt->h.Length - sizeof(xsdt->h)) / 8;
		for (int i=0; i<entries; i++){
			ACPISDTHeader* h = (ACPISDTHeader*) physical_to_virtual((uint64_t)(&rsdt->PointerToOtherSDT)[i]);
			if (!strncmp(h->Signature, "APIC", 4)){
				void* addr;
				if ((addr = check_madt_type(h, type))!=NULL){
					if (ccount == 0)
						return (void*)addr;
					else
						--ccount;
				}
			}
		}
	}
	return NULL;
}

bool check_lowestbyte(uintmax_t value) {
	char* ch = (char*) &value;
	return ch[0] == 0;
}

bool valid_rsdp(struct RSDPDescriptor* rsdp) {
	acpi_version = rsdp->Revision+1;
	if (acpi_version == 1){
		return check_lowestbyte(add_bytes((uint8_t*)rsdp, sizeof(struct RSDPDescriptor)));
	} else {
		return check_lowestbyte(add_bytes((uint8_t*)rsdp, sizeof(struct RSDPDescriptor20)));
	}
}

struct RSDPDescriptor* find_rsdp() {
	struct RSDPDescriptor* desc;
	uint64_t test_addr = (uint64_t)ebda;
	for (uint64_t addr = test_addr; addr < test_addr+0x1000; addr += 16){
		char* test_address = (char*)physical_to_virtual(addr);
		if (strncmp(test_address, "RSD PTR ", 8)==0){
			desc = (struct RSDPDescriptor*)test_address;
			if (valid_rsdp(desc))
				return desc;
		}
	}
	test_addr = 0x000E0000;
	for (uint64_t addr = test_addr; addr < 0x000FFFFF; addr += 16){
		char* test_address = (char*)physical_to_virtual(addr);
		if (strncmp(test_address, "RSD PTR ", 8)==0){
			desc = (struct RSDPDescriptor*)test_address;
			if (valid_rsdp(desc))
				return desc;
		}
	}

	return NULL;
}

bool valid_fadt(FADT* fadt) {
	return acpisdt_checksum((ACPISDTHeader*)fadt);
}

void init_table_acpi() {
	rsdp_descriptor = NULL;
	rsdt = NULL;
	xsdt = NULL;
	fadt = NULL;
	acpi_version = 0;

	rsdp_descriptor = find_rsdp();

	if (rsdp_descriptor == NULL)
		return;

	if (acpi_version == 1){
		RSDT* rsdtp = (RSDT*)physical_to_virtual((uint64_t)rsdp_descriptor->RsdtAddress);
		if (valid_rsdt(rsdtp)){
			rsdt = rsdtp;
		} else {
			return;
		}
	} else {
		XSDT* xsdtp = (XSDT*)physical_to_virtual(((struct RSDPDescriptor20*)rsdp_descriptor)->XsdtAddress);
		if (valid_xsdt(xsdtp)){
			xsdt = xsdtp;
		} else {
			return;
		}
	}

	void* fadt_address;

	if (acpi_version == 1)
		fadt_address = findFACP(rsdt);
	else
		fadt_address = findFACP_XSDT(xsdt);

	if (valid_fadt((FADT*)fadt_address)){
		fadt = (FADT*)fadt_address;
	}
}
