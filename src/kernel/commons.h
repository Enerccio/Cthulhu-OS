/*
 * The MIT License (MIT)
 * Copyright (c) 2015 Peter Vanusanik <admin@en-circle.com>
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation the
 * rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is furnished
 * to do so, subject to the following conditions:
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
 * commons.h
 *  Created on: Dec 22, 2015
 *      Author: Peter Vanusanik
 *  Contents: base kernel information
 */
#pragma once

#define KERNEL_MAIN_VERSION  "0"
#define KERNEL_MINOR_VESION  "0"
#define KERNEL_BUILD_VERSION "1"
#define KERNEL_CODENAME      "Blasting Mangoo"

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include "utils/textinput.h"
#include "utils/logger.h"

#define MULTIBOOT_FLAG_MEM     0x001
#define MULTIBOOT_FLAG_DEVICE  0x002
#define MULTIBOOT_FLAG_CMDLINE 0x004
#define MULTIBOOT_FLAG_MODS    0x008
#define MULTIBOOT_FLAG_AOUT    0x010
#define MULTIBOOT_FLAG_ELF     0x020
#define MULTIBOOT_FLAG_MMAP    0x040
#define MULTIBOOT_FLAG_CONFIG  0x080
#define MULTIBOOT_FLAG_LOADER  0x100
#define MULTIBOOT_FLAG_APM     0x200
#define MULTIBOOT_FLAG_VBE     0x400

struct multiboot
{
   uint32_t flags;
   uint32_t mem_lower;
   uint32_t mem_upper;
   uint32_t boot_device;
   uint32_t cmdline;
   uint32_t mods_count;
   uint32_t mods_addr;
   uint32_t num;
   uint32_t size;
   uint32_t addr;
   uint32_t shndx;
   uint32_t mmap_length;
   uint32_t mmap_addr;
   uint32_t drives_length;
   uint32_t drives_addr;
   uint32_t config_table;
   uint32_t boot_loader_name;
   uint32_t apm_table;
   uint32_t vbe_control_info;
   uint32_t vbe_mode_info;
   uint32_t vbe_mode;
   uint32_t vbe_interface_seg;
   uint32_t vbe_interface_off;
   uint32_t vbe_interface_len;
}  __attribute__((packed));

typedef struct multiboot_header multiboot_header_t;

#define ENABLE_INTERRUPTS() do { __asm __volatile__ ("sti"); } while (0)
#define DISABLE_INTERRUPTS() do { __asm __volatile__ ("cli"); } while (0)

typedef int8_t cmpresult_t;

#define UNSAFE_THREAD_CALL
#define SINGLETON_RETURN

#ifndef ALIGN_AMOUNT
#define ALIGN_AMOUNT 4096
#endif

typedef void* aligned_ptr_t;

#define RESERVED_KBLOCK_REVERSE_MAPPINGS (510UL)
#define RESERVED_KBLOCK_RAM_MAPPINGS     (509UL)
#define RESERVED_KBLOCK_KHEAP_MAPPINGS   (508UL)

#define ADDRESS_OFFSET(offset) (0xFFFF000000000000UL + (offset<<39))
