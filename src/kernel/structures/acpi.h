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
 * rsdt.h
 *  Created on: Dec 24, 2015
 *      Author: Peter Vanusanik
 *  Contents: 
 */

#pragma once

#include "../commons.h"
#include "../memory/paging.h"

#include <string.h>

struct RSDPDescriptor {
	char Signature[8];
	uint8_t Checksum;
	char OEMID[6];
	uint8_t Revision;
	uint32_t RsdtAddress;
} __attribute__ ((packed));

struct RSDPDescriptor20 {
	struct RSDPDescriptor firstPart;

	uint32_t Length;
	uint64_t XsdtAddress;
	uint8_t ExtendedChecksum;
	uint8_t reserved[3];
} __attribute__ ((packed));

typedef struct ACPISDTHeader {
	char Signature[4];
	uint32_t Length;
	uint8_t Revision;
	uint8_t Checksum;
	char OEMID[6];
	char OEMTableID[8];
	uint32_t OEMRevision;
	uint32_t CreatorID;
	uint32_t CreatorRevision;
} ACPISDTHeader;

typedef struct GenericAddressStructure
{
	uint8_t AddressSpace;
	uint8_t BitWidth;
	uint8_t BitOffset;
	uint8_t AccessSize;
	uint64_t Address;
} GenericAddressStructure;

typedef struct FADT
{
	struct   ACPISDTHeader h;
	uint32_t FirmwareCtrl;
	uint32_t Dsdt;

	// field used in ACPI 1.0; no longer in use, for compatibility only
	uint8_t  Reserved;

	uint8_t  PreferredPowerManagementProfile;
	uint16_t SCI_Interrupt;
	uint32_t SMI_CommandPort;
	uint8_t  AcpiEnable;
	uint8_t  AcpiDisable;
	uint8_t  S4BIOS_REQ;
	uint8_t  PSTATE_Control;
	uint32_t PM1aEventBlock;
	uint32_t PM1bEventBlock;
	uint32_t PM1aControlBlock;
	uint32_t PM1bControlBlock;
	uint32_t PM2ControlBlock;
	uint32_t PMTimerBlock;
	uint32_t GPE0Block;
	uint32_t GPE1Block;
	uint8_t  PM1EventLength;
	uint8_t  PM1ControlLength;
	uint8_t  PM2ControlLength;
	uint8_t  PMTimerLength;
	uint8_t  GPE0Length;
	uint8_t  GPE1Length;
	uint8_t  GPE1Base;
	uint8_t  CStateControl;
	uint16_t WorstC2Latency;
	uint16_t WorstC3Latency;
	uint16_t FlushSize;
	uint16_t FlushStride;
	uint8_t  DutyOffset;
	uint8_t  DutyWidth;
	uint8_t  DayAlarm;
	uint8_t  MonthAlarm;
	uint8_t  Century;

	// reserved in ACPI 1.0; used since ACPI 2.0+
	uint16_t BootArchitectureFlags;

	uint8_t  Reserved2;
	uint32_t Flags;

	// 12 byte structure; see below for details
	GenericAddressStructure ResetReg;

	uint8_t  ResetValue;
	uint8_t  Reserved3[3];

	// 64bit pointers - Available on ACPI 2.0+
	uint64_t                X_FirmwareControl;
	uint64_t                X_Dsdt;

	GenericAddressStructure X_PM1aEventBlock;
	GenericAddressStructure X_PM1bEventBlock;
	GenericAddressStructure X_PM1aControlBlock;
	GenericAddressStructure X_PM1bControlBlock;
	GenericAddressStructure X_PM2ControlBlock;
	GenericAddressStructure X_PMTimerBlock;
	GenericAddressStructure X_GPE0Block;
	GenericAddressStructure X_GPE1Block;
} FADT;

typedef struct RSDT {
	struct ACPISDTHeader h;
	uint32_t PointerToOtherSDT;//[(h.Length - sizeof(h)) / 4];
} RSDT;

typedef struct XSDT {
	struct ACPISDTHeader h;
	uint64_t PointerToOtherSDT;//[(h.Length - sizeof(h)) / 8];
} XSDT;

typedef struct MADT_HEADER {
	ACPISDTHeader header;
	uint32_t address;
	uint32_t flags;
} MADT_HEADER;

enum acpi_madt_type {
	ACPI_MADT_TYPE_LOCAL_APIC = 0,
	ACPI_MADT_TYPE_IO_APIC = 1,
	ACPI_MADT_TYPE_INTERRUPT_OVERRIDE = 2,
	ACPI_MADT_TYPE_NMI_SOURCE = 3,
	ACPI_MADT_TYPE_LOCAL_APIC_NMI = 4,
	ACPI_MADT_TYPE_LOCAL_APIC_OVERRIDE = 5,
	ACPI_MADT_TYPE_IO_SAPIC = 6,
	ACPI_MADT_TYPE_LOCAL_SAPIC = 7,
	ACPI_MADT_TYPE_INTERRUPT_SOURCE = 8,
	ACPI_MADT_TYPE_LOCAL_X2APIC = 9,
	ACPI_MADT_TYPE_LOCAL_X2APIC_NMI = 10,
	ACPI_MADT_TYPE_GENERIC_INTERRUPT = 11,
	ACPI_MADT_TYPE_GENERIC_DISTRIBUTOR = 12,
	ACPI_MADT_TYPE_GENERIC_MSI_FRAME = 13,
	ACPI_MADT_TYPE_GENERIC_REDISTRIBUTOR = 14,
	ACPI_MADT_TYPE_GENERIC_TRANSLATOR = 15,
	ACPI_MADT_TYPE_RESERVED = 16
};

typedef struct ACPI_SUBTABLE_HEADER {
	uint8_t type;
	uint8_t length;
} ACPI_SUBTABLE_HEADER;

typedef struct MADT_LOCAL_APIC {
	struct ACPI_SUBTABLE_HEADER header;
	uint8_t  processor_id;
	uint8_t  id;
	uint32_t lapic_flags;
} MADT_LOCAL_APIC;

extern struct RSDPDescriptor* rsdp_descriptor;
extern uint8_t acpi_version;
extern RSDT* rsdt;
extern XSDT* xsdt;
extern FADT* fadt;

void init_table_acpi();
void* find_madt(enum acpi_madt_type type, unsigned int ccount);
