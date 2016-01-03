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

/** RSDP descriptor or NULL if none is available */
struct RSDPDescriptor* rsdp_descriptor;
/** RSDT table or NULL if none is available */
RSDT*   rsdt;
/** XSDT table or NULL if none is available */
XSDT*   xsdt;
/** FADT table or NULL if none is available */
FADT*   fadt;
/** ACPI version is stored here */
uint8_t acpi_version;

extern void* ebda;

/**
 * Adds all bytes of target address together to one number.
 */
uintmax_t add_bytes(uint8_t* addr, size_t len) {
    uintmax_t v = 0;
    for (size_t p = 0; p < len ; p++)
        v += addr[p];
    return v;
}

/**
 * Performs acpisdt checksum for header.
 */
bool acpisdt_checksum(ACPISDTHeader* th) {
    unsigned char sum = 0;

    for (unsigned int i = 0; i < th->Length; i++) {
        sum += ((char *) th)[i];
    }

    return sum == 0;
}

/**
 * Checks whether rsdt is valid
 */
bool valid_rsdt(RSDT* rsdt) {
    return acpisdt_checksum((ACPISDTHeader*)rsdt);
}

/**
 * Checks whether xsdt is valid
 */
bool valid_xsdt(XSDT* xsdt) {
    return acpisdt_checksum((ACPISDTHeader*)xsdt);
}

/**
 * Finds FACP table or returns NULL.
 *
 * Argument must be RSDT table.
 */
void* findFACP(void* RootSDT) {
    RSDT* rsdt = (RSDT*) RootSDT;
    int entries = (rsdt->h.Length - sizeof(rsdt->h)) / 4;

    for (int i=0; i<entries; i++) {
        ACPISDTHeader* h = (ACPISDTHeader*) physical_to_virtual((uintptr_t)(&rsdt->PointerToOtherSDT)[i]);
        if (!strncmp(h->Signature, "FACP", 4))
            return (void*)h;
    }

    // No FACP found
    return NULL;
}

/**
 * Finds FACP table or returns NULL.
 *
 * Argument must be XSDT table.
 */
void* findFACP_XSDT(void* RootSDT) {
    XSDT* xsdt = (XSDT*) RootSDT;
    int entries = (xsdt->h.Length - sizeof(xsdt->h)) / 8;

    for (int i=0; i<entries; i++) {
        ACPISDTHeader* h = (ACPISDTHeader*) physical_to_virtual((uintptr_t)(&rsdt->PointerToOtherSDT)[i]);
        if (!strncmp(h->Signature, "FACP", 4))
            return (void*)h;
    }

    // No FACP found
    return NULL;
}

/**
 * Returns MADT table or NULL if it does not exist.
 *
 * Searches either RSDT or XSDT for MADT table and returns it.
 */
void* find_madt() {
    if (acpi_version == 1) {
        if (rsdt == NULL)
            return NULL;
        int entries = (rsdt->h.Length - sizeof(rsdt->h)) / 4;
        for (int i=0; i<entries; i++) {
            ACPISDTHeader* h = (ACPISDTHeader*) physical_to_virtual((uintptr_t)(&rsdt->PointerToOtherSDT)[i]);
            if (!strncmp(h->Signature, "APIC", 4)) {
                return h;
            }
        }
    } else {
        if (xsdt == NULL)
            return NULL;
        int entries = (xsdt->h.Length - sizeof(xsdt->h)) / 8;
        for (int i=0; i<entries; i++) {
            ACPISDTHeader* h = (ACPISDTHeader*) physical_to_virtual((uintptr_t)(&xsdt->PointerToOtherSDT)[i]);
            if (!strncmp(h->Signature, "APIC", 4)) {
                return h;
            }
        }
    }
    return NULL;
}

/**
 * Checks whether lowest byte of value is 0.
 */
bool check_lowestbyte(uintmax_t value) {
    char* ch = (char*) &value;
    return ch[0] == 0;
}

/**
 * Checks whether RSDP is valid.
 */
bool valid_rsdp(struct RSDPDescriptor* rsdp) {
    acpi_version = rsdp->Revision+1;
    if (acpi_version == 1) {
        return check_lowestbyte(add_bytes((uint8_t*)rsdp, sizeof(struct RSDPDescriptor)));
    } else {
        return check_lowestbyte(add_bytes((uint8_t*)rsdp, sizeof(struct RSDPDescriptor20)));
    }
}

/**
 * Returns pointer to RSDP or NULL if none can be found.
 *
 * Searches whole ebda or address from 0x000E0000 to 0x000FFFFF for
 * string "RSD PTR " at align borders (0xF align), if found,
 * creates RSDP from that memory location and checks for validity.
 * If valid, it returns it, otherwise continues the search loop.
 */
struct RSDPDescriptor* find_rsdp() {
    struct RSDPDescriptor* desc;
    uintptr_t test_addr = (uintptr_t)ebda;
    for (uintptr_t addr = test_addr; addr < test_addr+0x1000; addr += 16) {
        char* test_address = (char*)addr;
        if (strncmp(test_address, "RSD PTR ", 8)==0) {
            desc = (struct RSDPDescriptor*)test_address;
            if (valid_rsdp(desc))
                return desc;
        }
    }
    test_addr = 0x000E0000;
    for (uintptr_t addr = test_addr; addr < 0x000FFFFF; addr += 16) {
        char* test_address = (char*)addr;
        if (strncmp(test_address, "RSD PTR ", 8)==0) {
            desc = (struct RSDPDescriptor*)test_address;
            if (valid_rsdp(desc))
                return desc;
        }
    }

    return NULL;
}

/**
 * Checks whether fadt is valid or not.
 */
bool valid_fadt(FADT* fadt) {
    return acpisdt_checksum((ACPISDTHeader*)fadt);
}

/**
 * Initializes ACPI tables.
 *
 * Finds RSDP descriptor, then determines ACPI version and
 * based on that finds either RSDT or XSDT. If those are valid,
 * it searches them for FADT table and saves it.
 *
 * If any of those steps fail, sets remaining structures to NULL.
 */
void init_table_acpi() {
    rsdp_descriptor = NULL;
    rsdt = NULL;
    xsdt = NULL;
    fadt = NULL;
    acpi_version = 0;

    rsdp_descriptor = find_rsdp();

    if (rsdp_descriptor == NULL) {
        log_warn("No RSDP Table detected");
        return;
    }

    if (acpi_version == 1) {
        RSDT* rsdtp = (RSDT*)physical_to_virtual((uintptr_t)rsdp_descriptor->RsdtAddress);
        if (valid_rsdt(rsdtp)) {
            rsdt = rsdtp;
        } else {
            log_warn("No RSDT Table detected");
            return;
        }
    } else {
        XSDT* xsdtp = (XSDT*)physical_to_virtual(((struct RSDPDescriptor20*)rsdp_descriptor)->XsdtAddress);
        if (valid_xsdt(xsdtp)) {
            xsdt = xsdtp;
        } else {
            log_warn("No XSDT Table detected");
            return;
        }
    }

    void* fadt_address;

    if (acpi_version == 1)
        fadt_address = findFACP(rsdt);
    else
        fadt_address = findFACP_XSDT(xsdt);

    if (valid_fadt((FADT*)fadt_address)) {
        fadt = (FADT*)fadt_address;
    } else {
        log_warn("No FADT Table detected");
    }
}
