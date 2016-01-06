/*
 * The MIT License (MIT)
 * Copyright (c) 2015 Peter Vanusanik <admin@en-circle.com>
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights to
 * use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of
 * the Software, and to permit persons to whom the Software is furnished to do so,
 * subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED,
 * INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A
 * PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
 * HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION
 * OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE
 * OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 *
 * paging.c
 *  Created on: Dec 22, 2015
 *      Author: Peter Vanusanik
 *  Contents: paging memory for x86-64
 */
#include "paging.h"
#include "heap.h"
#include "../cpus/ipi.h"
#include "../cpus/cpu_mgmt.h"

/** aligns the address to 0x1000 and then casts it to type */
#define PALIGN(type, addr) ((type)ALIGN(addr))


/** Maximum physical bits in page structures */
uint64_t maxphyaddr;
/** Maximum ram address, might be unusable */
uint64_t maxram;

section_info_t* frame_pool;

bool __mem_mirror_present;
struct multiboot_info multiboot_info;
ruint_t __frame_lock;

extern uint64_t detect_maxphyaddr();
extern uint64_t get_active_page();
extern void set_active_page(puint_t address);
extern ruint_t is_1GB_paging_supported();
extern void invalidate_address(void* addr);
extern uint16_t* text_mode_video_memory;
extern void*     ebda;
extern puint_t kernel_tmp_heap_start;
extern unsigned char _frame_block[0];
extern void proc_spinlock_lock(void* address);
extern void proc_spinlock_unlock(void* address);
extern void kp_halt();

/**
 * Virtual address structure for standard paging.
 */
typedef struct v_address {
    uint64_t offset :12;
    uint64_t table :9;
    uint64_t directory :9;
    uint64_t directory_ptr :9;
    uint64_t pml :9;
    uint64_t rest :16;
} v_address_t;

/**
 * Virtual address structure for 1GB page.
 */
typedef struct v_address1GB {
    uint64_t offset :30;
    uint64_t directory_ptr :9;
    uint64_t pml :9;
    uint64_t rest :16;
} v_address1GB_t;

#define MMU_RECURSIVE_SLOT      (RESERVED_KBLOCK_REVERSE_MAPPINGS)

// Convert an address into array index of a structure
// E.G. int index = MMU_PML4_INDEX(0xFFFFFFFFFFFFFFFF); // index = 511
#define MMU_PML4_INDEX(addr)    ((((uintptr_t)(addr))>>39) & 511)
#define MMU_PDPT_INDEX(addr)    ((((uintptr_t)(addr))>>30) & 511)
#define MMU_PD_INDEX(addr)      ((((uintptr_t)(addr))>>21) & 511)
#define MMU_PT_INDEX(addr)      ((((uintptr_t)(addr))>>12) & 511)

// Base address for paging structures
#define KADDR_MMU_PT            (0xFFFF000000000000UL + (MMU_RECURSIVE_SLOT<<39))
#define KADDR_MMU_PD            (KADDR_MMU_PT         + (MMU_RECURSIVE_SLOT<<30))
#define KADDR_MMU_PDPT          (KADDR_MMU_PD         + (MMU_RECURSIVE_SLOT<<21))
#define KADDR_MMU_PML4          (KADDR_MMU_PDPT       + (MMU_RECURSIVE_SLOT<<12))

// Structures for given address, for example
// uint64_t* pt = MMU_PT(addr)
// uint64_t physical_addr = pt[MMU_PT_INDEX(addr)];
#define MMU_PML4(addr)          ((uint64_t*)  KADDR_MMU_PML4 )
#define MMU_PDPT(addr)          ((uint64_t*)( KADDR_MMU_PDPT + (((addr)>>27) & 0x00001FF000) ))
#define MMU_PD(addr)            ((uint64_t*)( KADDR_MMU_PD   + (((addr)>>18) & 0x003FFFF000) ))
#define MMU_PT(addr)            ((uint64_t*)( KADDR_MMU_PT   + (((addr)>>9)  & 0x7FFFFFF000) ))

/** Checks if page structure at addr is present or not */
#define PRESENT(addr) (((uint64_t)addr) & 1)

/**
 * Computes virtual to physical mapping.
 *
 * Checks for page hierarchy and determines what physical address will be.
 * If page structures are missing on the way to decode, physical address cannot
 * be found and this valid will contain 0, otherwise valid will contain 1
 *
 * 1GB page sector (ram identity map) behaves slightly different, because it
 * needs different v_address_t type.
 */
ruint_t virtual_to_physical(uintptr_t vaddress, uint8_t* valid) {
    *valid = 1;
    v_address_t virtual_address = *((v_address_t*) &vaddress);

    uint64_t* address = (uint64_t*) MMU_PML4(vaddress)[MMU_PML4_INDEX(vaddress)];

    if (!PRESENT(address)) {
        *valid = 0;
        return 0;
    }

    address = (uint64_t*) MMU_PDPT(vaddress)[MMU_PDPT_INDEX(vaddress)];

    if (!PRESENT(address)) {
        *valid = 0;
        return 0;
    }

    if (vaddress > ADDRESS_OFFSET(RESERVED_KBLOCK_RAM_MAPPINGS) &&
                vaddress < ADDRESS_OFFSET((RESERVED_KBLOCK_RAM_MAPPINGS+1)) &&
                is_1GB_paging_supported()) {
        v_address1GB_t va = *((v_address1GB_t*) &vaddress);
        page_directory1GB_t pd1gb;
        pd1gb.number = (uint64_t) address;
        return pd1gb.flaggable.address + va.offset;
    }

    address = (uint64_t*) MMU_PD(vaddress)[MMU_PD_INDEX(vaddress)];

    if (!PRESENT(address)) {
        *valid = 0;
        return 0;
    }

    ruint_t physadd = MMU_PT(vaddress)[MMU_PT_INDEX(vaddress)];
    return ALIGN(physadd) + virtual_address.offset;
}

/**
 * Returns address + physical identity map offset
 */
uintptr_t physical_to_virtual(ruint_t paddress) {
    if (!__mem_mirror_present)
        return paddress;
    if (paddress == 0)
        return 0;
    return paddress + ADDRESS_OFFSET(RESERVED_KBLOCK_RAM_MAPPINGS);
}

/**
 * Always returns free frame
 * TODO: add paging to swap
 */
extern void kp_halt();
static puint_t get_free_frame() {
    if (frame_pool == NULL) {
        return ((puint_t)malign(0x1000, 0x1000)-0xFFFFFFFF80000000);
    } else {
        section_info_t* section = (section_info_t*)physical_to_virtual((puint_t)frame_pool);
        while (section != NULL) {
            if (section->head != NULL) {
                stack_element_t* se = (stack_element_t*) physical_to_virtual((puint_t)section->head);
                section->head = se->next;
                ((frame_info_t*)physical_to_virtual((puint_t)section->frame_array))
                        [se->array_ord].usage_count = 1;
                return se->frame_address;
            }
            section = (section_info_t*)physical_to_virtual((puint_t)section->next_section);
        }

        proc_spinlock_unlock(&__frame_lock);
        // TODO handle crap here
        kp_halt();
        return 0;
    }
}

/**
 * Deallocates frame from frame_map.
 */
static void free_frame(puint_t frame_address) {
    puint_t fa = ALIGN(frame_address);

    section_info_t* section = (section_info_t*)physical_to_virtual((puint_t)frame_pool);
    while (section != NULL) {
        if (section->start_word >= fa && section->end_word < fa) {
            uint32_t idx = (fa-section->start_word) / 0x1000;
            frame_info_t* fi = (frame_info_t*)physical_to_virtual((puint_t)section->frame_array);
            --fi[idx].usage_count;
            if (fi[idx].cow_count > 0)
                --fi[idx].cow_count;
            if (fi[idx].usage_count == 0) {
                fi[idx].bound_stack_element->next = section->head;
                section->head = fi[idx].bound_stack_element;
                memset((void*)physical_to_virtual(
                        ((stack_element_t*)physical_to_virtual((puint_t)section->head))->frame_address),
                        0xDE, 0x1000);
                return;
            }
        } else {
            section = (section_info_t*)physical_to_virtual((puint_t)section->next_section);
        }
    }
    // unmapped address not in a pool, most likely <2MB.
}

static frame_info_t* get_frame_info(puint_t fa) {

    section_info_t* section = (section_info_t*)physical_to_virtual((puint_t)frame_pool);
    while (section != NULL) {
        if (section->start_word <= fa && section->end_word > fa) {
            uint32_t idx = (fa-section->start_word) / 0x1000;
            frame_info_t* fi = (frame_info_t*)physical_to_virtual((puint_t)section->frame_array);
            return &fi[idx];
        } else {
            section = (section_info_t*)physical_to_virtual((puint_t)section->next_section);
        }
    }

    return NULL;
}

/**
 * Returns page allocated for that virtual address.
 *
 * If allocate_new is specified to true, it will allocate substructures,
 * if not present.Otherwise returns NULL if page structures are not present
 * on the way to the virtual address. Returned pointer points to page in page
 * table.
 */
static puint_t* __get_page(uintptr_t vaddress, bool allocate_new, bool user) {
    puint_t* pdpt_addr = (uint64_t*) MMU_PML4(vaddress)[MMU_PML4_INDEX(
            vaddress)];

    if (!PRESENT(pdpt_addr)) {
        if (!allocate_new)
            return 0;
        pdpt_t pdpt;
        memset(&pdpt, 0, sizeof(pdpt_t));
        proc_spinlock_lock(&__frame_lock);
        pdpt.number = get_free_frame();
        proc_spinlock_unlock(&__frame_lock);
        pdpt.flaggable.present = 1;
        pdpt.flaggable.us = user;
        pdpt.flaggable.rw = 1;
        MMU_PML4(vaddress)[MMU_PML4_INDEX(vaddress)] = pdpt.number;
        memset(MMU_PDPT(vaddress), 0, 0x1000);
    }

    uint64_t* pdir_addr = (uint64_t*) MMU_PDPT(vaddress)[MMU_PDPT_INDEX(
            vaddress)];

    if (!PRESENT(pdir_addr)) {
        if (!allocate_new)
            return 0;
        page_directory_t dir;
        memset(&dir, 0, sizeof(page_directory_t));
        proc_spinlock_lock(&__frame_lock);
        dir.number = get_free_frame();
        proc_spinlock_unlock(&__frame_lock);
        dir.flaggable.present = 1;
        dir.flaggable.us = user;
        dir.flaggable.rw = 1;
        MMU_PDPT(vaddress)[MMU_PDPT_INDEX(vaddress)] = dir.number;
        memset(MMU_PD(vaddress), 0, 0x1000);
    }

    uint64_t* pt_addr = (uint64_t*) MMU_PD(vaddress)[MMU_PD_INDEX(vaddress)];

    if (!PRESENT(pt_addr)) {
        if (!allocate_new)
            return 0;
        page_table_t pt;
        memset(&pt, 0, sizeof(page_table_t));
        proc_spinlock_lock(&__frame_lock);
        pt.number = get_free_frame();
        proc_spinlock_unlock(&__frame_lock);
        pt.flaggable.present = 1;
        pt.flaggable.us = user;
        pt.flaggable.rw = 1;
        MMU_PD(vaddress)[MMU_PD_INDEX(vaddress)] = pt.number;
        memset(MMU_PT(vaddress), 0, 0x1000);
    }

    return (puint_t*)&MMU_PT(vaddress)[MMU_PT_INDEX(vaddress)];
}

static puint_t* get_page(uintptr_t vaddress, bool allocate_new) {
	return __get_page(vaddress, allocate_new, false);
}

static puint_t* get_page_user(uintptr_t vaddress, bool allocate_new) {
	return __get_page(vaddress, allocate_new, true);
}

void free_page_structure(uintptr_t vaddress) {
    uint64_t* pdpt_addr = (uint64_t*) MMU_PML4(vaddress)[MMU_PML4_INDEX(
                vaddress)];

    if (!PRESENT(pdpt_addr))
        return;

    uint64_t* pdir_addr = (uint64_t*) MMU_PDPT(vaddress)[MMU_PDPT_INDEX(
                vaddress)];

    if (!PRESENT(pdir_addr))
        return;

    uint64_t* pt_addr = (uint64_t*) MMU_PD(vaddress)[MMU_PD_INDEX(vaddress)];

    if (!PRESENT(pt_addr))
        return;

    MMU_PT(vaddress)[MMU_PT_INDEX(vaddress)] = 0; // free table entry

    // check for other table entries
    uint64_t* pt_virt = (uint64_t*)physical_to_virtual(ALIGN((uint64_t)pt_addr));
    for (size_t i=0; i<512; i++) {
        if (pt_virt[i] != 0)
            return;
    }

    free_frame(ALIGN(MMU_PD(vaddress)[MMU_PD_INDEX(vaddress)]));
    MMU_PD(vaddress)[MMU_PD_INDEX(vaddress)] = 0; // free pt address

    // check for other pdir adresses
    uint64_t* pdir_virt = (uint64_t*)physical_to_virtual(ALIGN((uint64_t)pdir_addr));
    for (size_t i=0; i<512; i++) {
        if (pdir_virt[i] != 0)
            return;
    }

    free_frame(ALIGN(MMU_PDPT(vaddress)[MMU_PDPT_INDEX(vaddress)]));
    MMU_PDPT(vaddress)[MMU_PDPT_INDEX(vaddress)] = 0; // free pdir address

    uint64_t* pdpt_virt = (uint64_t*)physical_to_virtual(ALIGN((uint64_t)pdpt_addr));
    for (size_t i=0; i<512; i++) {
        if (pdpt_virt[i] != 0)
            return;
    }

    free_frame(ALIGN(MMU_PML4(vaddress)[MMU_PML4_INDEX(vaddress)]));
    MMU_PML4(vaddress)[MMU_PML4_INDEX(vaddress)] = 0; // free pdpt address
}

void deallocate_start_memory() {
    memset((void*)physical_to_virtual((uintptr_t)get_active_page()), 0, 256*sizeof(uintptr_t));
    broadcast_ipi_message(true, IPI_INVALIDATE_PAGE, 0, 0x200000, NULL);
}

/**
 * Multiboot memory structure
 */
typedef struct
    __attribute__((packed)) {
        uint64_t address;
        uint64_t size;
        uint32_t type;
    } mboot_mem_t;

/**
 * Detects maximum ram, as provided by multiboot.
 *
 * Searches multiboot structure for memory entries, and then
 * returns total addressable space (might contain special
 * addresses).
 */
static uint64_t detect_maxram(struct multiboot_info* mboot_addr) {
    uint64_t highest = 0;
    if ((mboot_addr->flags & 0b1) == 1) {
        highest = mboot_addr->mem_lower * 1024;
        if ((mboot_addr->mem_upper * 1024) > highest)
            highest = mboot_addr->mem_upper * 1024;
    }

    if ((mboot_addr->flags & 0b100000) == 0b100000) {
        uint32_t mem = mboot_addr->mmap_addr + 4;
        while (mem < (mboot_addr->mmap_addr + mboot_addr->mmap_length)) {
            mboot_mem_t data = *((mboot_mem_t*) (uint64_t) mem);
            uint32_t size = *((uint32_t*) (uint64_t) (mem - 4));
            mem += size + 4;

            uint64_t base_addr = data.address;
            uint64_t length = data.size;

            uint64_t totalsize = base_addr + length;
            if (totalsize > highest)
                highest = totalsize;
        }
    }

    if (highest == 0) {
        kp_halt();
    }

    return highest;
}

static void* remap(puint_t phys_address) {
    if (phys_address < 0x200000)
        return (void*)phys_address;

    uintptr_t map_address = (uintptr_t)_frame_block;
    puint_t* paddress = get_page(map_address, false);

    page_t page;
    memset(&page, 0, sizeof(page_t));
    page.address = ALIGN(phys_address);
    page.flaggable.present = 1;
    page.flaggable.rw = 1;
    page.flaggable.us = 0;
    *paddress = page.address;

    paddress = get_page(map_address+0x1000, false);
    memset(&page, 0, sizeof(page_t));
    page.address = ALIGN(phys_address) + 0x1000;
    page.flaggable.present = 1;
    page.flaggable.rw = 1;
    page.flaggable.us = 0;
    *paddress = page.address;

    invalidate_address((void*)map_address);
    invalidate_address((void*)(map_address+0x1000));

    return (void*)(map_address+(phys_address-ALIGN(phys_address)));
}

size_t __strlen(char* c) {
    size_t len = 0;
    do {
        if (len % 0x1000 == 0) {
            c = (char*)remap((puint_t)c);
        }
        if (*c++ == '\0')
            break;
        else
            ++len;
    } while (true);
    return len;
}

bool check_used_range(puint_t test, puint_t fa, size_t sa) {
    ruint_t from = ALIGN(fa);
    sa += fa - from;
    ruint_t size = ALIGN_UP(sa);
    if (from <= test && test < from + size) {
        return true;
    }
    return false;
}

bool check_if_used_string(puint_t test, char* string) {
    return check_used_range(test, (puint_t)string, __strlen(string)+1);
}

bool check_if_used(struct multiboot_info* mbheader, puint_t test) {
    puint_t kend = kernel_tmp_heap_start - 0xFFFFFFFF80000000 + 0x40000;
    if (check_used_range(test, 0, kend))
        return true;

    if (check_used_range(test, (puint_t)mbheader, sizeof(struct multiboot_info)))
        return true;

    char* cmdline = (char*)(puint_t)mbheader->cmdline;
    if (check_if_used_string(test, cmdline))
        return true;

    size_t mods_size = mbheader->mods_count * sizeof(struct multiboot_mod_list);
    if (check_used_range(test, mbheader->mods_addr, mods_size))
        return true;

    struct multiboot_mod_list* modules = (struct multiboot_mod_list*) (puint_t) mbheader->mods_addr;
    for (uint32_t i=0; i<mbheader->mods_count; i++) {
        struct multiboot_mod_list* module = (struct multiboot_mod_list*)remap((puint_t)&modules[i]);
        size_t mod_size = module->mod_end-module->mod_start;
        if (check_used_range(test, (puint_t)module->mod_start, mod_size))
            return true;
        char* mod_name = (char*)(puint_t)module->cmdline;
        if (check_if_used_string(test, mod_name))
            return true;
    }

    if ((mbheader->flags & (1<<11)) != 0 && (mbheader->flags & (1<12)) != 0) {
        if (check_used_range(test, mbheader->framebuffer_addr,
                mbheader->framebuffer_bpp * mbheader->framebuffer_width * mbheader->framebuffer_height))
            return true;
        if (mbheader->framebuffer_type == 0)
            if (check_used_range(test, mbheader->framebuffer_palette_addr,
                    mbheader->framebuffer_palette_num_colors * 3))
                return true;
    }

    if (check_used_range(test, mbheader->mmap_addr, mbheader->mmap_length))
        return true;

    return false;
}

/**
 * Creates pool from bound adresses
 */
section_info_t* make_pool(size_t length, puint_t base_addr, section_info_t* lastfp, section_info_t** firstfp) {
    // total amount of frames available if we count in metadata
    size_t uframes = (length-sizeof(section_info_t))/(0x1000+(sizeof(stack_element_t)+sizeof(frame_info_t)))-1;
    // total size of metadata header
    size_t total_size = sizeof(section_info_t) + (uframes * sizeof(stack_element_t)) + (uframes * sizeof(frame_info_t));
    // address of first effective frame
    puint_t after_address = (base_addr + total_size + 0x1000) & ~0xFFF;

    // allocate base_addr and size of metadata header in virt. memory
    for (puint_t addr = base_addr; addr < base_addr + total_size; addr += 0x1000) {
        puint_t* paddress = get_page(addr, true);
        page_t page;
        memset(&page, 0, sizeof(page_t));
        page.address = addr;
        page.flaggable.present = 1;
        page.flaggable.rw = 1;
        page.flaggable.us = 1;
        *paddress = page.address;
    }

    // section starts at base address
    section_info_t* section = (section_info_t*)base_addr;
    if (lastfp != NULL) { // link previous section and this one
        lastfp->next_section = section;
    } else if (*firstfp == NULL) { // first section will be this one
        *firstfp = section;
    }

    // fill up section with information
    section->start_word = after_address;
    section->end_word = base_addr + length;
    section->total_frames = uframes;
    section->next_section = NULL;
    section->frame_array = (frame_info_t*) (base_addr + sizeof(section_info_t)); // starts at end of this structure
    puint_t stack_el_addr = (base_addr + sizeof(section_info_t) +
            (sizeof(frame_info_t) * uframes)); // first stack element describing the memory
    section->head = NULL;

    stack_element_t* prev_se = NULL;
    for (size_t i=0; i<uframes; i++) {
        // Fill up information for stack element.
        // Stack element contains frame address of free frame in this memory section
        stack_element_t* se = (stack_element_t*)stack_el_addr;
        stack_el_addr += sizeof(stack_element_t);
        se->frame_address = after_address + (i*0x1000);
        frame_info_t* fi = &section->frame_array[i];

        if (se->frame_address < 0x200000) {
            // "already used" stack element
            // so do not link this stack element at all
            // link stack elements together
            // fill up frame info with this stack element and number of times it has been used (0)

            fi->usage_count = 1;
        } else {
            // normal stack element
            // link stack elements together
            if (prev_se != NULL)
                prev_se->next = se;
            else
                section->head = se;
            se->next = NULL;
            se->array_ord = i;
            prev_se = se;

            // fill up frame info with this stack element and number of times it has been used (0)
            fi->usage_count = 0;
            fi->cow_count = 0;
        }
        fi->bound_stack_element = se;
    }

    return section;
}

void deallocate_starting_address(puint_t address, size_t size) {
    if (size < 0x10000)
        return; // ignore section and just leave it as dead memory
    section_info_t* se = frame_pool;
    while (se->next_section != NULL)
        se = se->next_section;
    make_pool(size, address, se, NULL);
}

/**
 * Creates frame map from multiboot structure.
 *
 * Fills bitmaps for frame_map_usage, phys_map and returns bitmap for frame_map
 * from available ram entries.
 */
static void create_frame_pool(struct multiboot_info* mboot_addr) {
    section_info_t* firstfp = NULL;
    section_info_t* lastfp = NULL;

    if ((mboot_addr->flags & 0b100000) == 0b100000) {
        uint32_t mem = mboot_addr->mmap_addr + 4;
        while (mem < (mboot_addr->mmap_addr + mboot_addr->mmap_length)) {
            mboot_mem_t data = *((mboot_mem_t*) (uint64_t) mem);
            uint32_t size = *((uint32_t*) (uint64_t) (mem - 4));
            mem += size + 4;

            puint_t ba = data.address;
            size_t len = data.size;

            if (data.type != 1) // not a ram
                continue;
            if (len < 0x10000)
                continue; // waste of a section

            puint_t base_addr = ba;
            size_t length = len;

            while (true) {
                // if we fail to find any useful starting address but there is more to search
                // restart the search here
start_search_again:;

                // search from current base_addr up to maximum in the block
                if (base_addr >= len+ba)
                    break; // end of search
                for (puint_t test = base_addr; test<len; test+=0x1000) {
                    if (check_if_used(mboot_addr, test)) {
                        // memory address is used by some internal structure
                        // write up how much size we have for this pool and then
                        // decide upon it
                        length = test - base_addr;
                        if (length == 0) {
                            // first 0x1000 already used
                            // check if there is more, if so, continue from 0x1000 onwards
                            // if not, this section is garbage
                            if (base_addr+0x1000<ba+len) {
                                base_addr += 0x1000;
                                goto start_search_again;
                            } else {
                                goto finish_this_section;
                            }
                        } else if (length < 0x10000) {
                            // pool with size <0x10000, same as above
                            // but add length, skipping good but too small pool
                            if (base_addr+length<ba+len) {
                                base_addr += length;
                                goto start_search_again;
                            } else {
                                goto finish_this_section;
                            }
                        }
                        // found a barrier, can only use this much in a block
                        goto use_length_found;
                    }
                }
                // no used address up to len, awesome, use this as a pool
                length = len;

use_length_found:;
                section_info_t* section = make_pool(length, base_addr, lastfp, &firstfp);

                lastfp = section;

                // increment base address by length
                base_addr += length;
            }
finish_this_section:;

        }

    }

    // we have our pool, so we set it and thus disable temp. heap allocation for page structures
    frame_pool = firstfp;
}

/**
 * Initializes memory mirror.
 *
 * Memory mirror allocates all available ram to specific range address.
 * If 1GB paging is supported, 1GB paging is used, otherwise only 4KB
 * paging is used, which means much more physical ram is used.
 */
void initialize_memory_mirror() {
    if (is_1GB_paging_supported() != 0) {
        for (puint_t start=0; start < maxram; start+=1<<30) {
            puint_t vaddress = start + ADDRESS_OFFSET(RESERVED_KBLOCK_RAM_MAPPINGS);
            puint_t* pdpt_addr = (puint_t*) MMU_PML4(vaddress)[MMU_PML4_INDEX(
                        vaddress)];

            if (!PRESENT(pdpt_addr)) {
                pdpt_t pdpt;
                proc_spinlock_lock(&__frame_lock);
                pdpt.number = get_free_frame();
                proc_spinlock_unlock(&__frame_lock);
                pdpt.flaggable.present = 1;
                MMU_PML4(vaddress)[MMU_PML4_INDEX(vaddress)] = pdpt.number;
                memset(MMU_PDPT(vaddress), 0, sizeof(uint64_t));
            }

            page_directory1GB_t dir;
            memset(&dir, 0, sizeof(page_directory1GB_t));
            dir.number = start;
            dir.flaggable.present = 1;
            dir.flaggable.ps = 1;
            dir.flaggable.rw = 1;
            MMU_PDPT(vaddress)[MMU_PDPT_INDEX(vaddress)] = dir.number;
        }
    } else {
        for (puint_t start=0; start < maxram; start+=0x1000) {
            puint_t vaddress = start + ADDRESS_OFFSET(RESERVED_KBLOCK_RAM_MAPPINGS);
            puint_t* paddress = get_page(vaddress, true);
            page_t page;
            memset(&page, 0, sizeof(page_t));
            page.address = start;
            page.flaggable.present = 1;
            page.flaggable.rw = 1;
            page.flaggable.us = 0;
            *paddress = page.address;
        }

    }
}

/**
 * Initializes correct paging.
 *
 * Performs it by these steps:
 *
 *  1. detects max ram via detect_maxram
 *  2. creates frame pool stack and array structure for all available ram
 *  3. detects maxphyaddr
 *  4. initializes memory mirror
 *  5- deallocates old memory (frames are not returned).
 */
void initialize_physical_memory_allocation(struct multiboot_info* mboot_addr) {
    __frame_lock = 0;

    struct multiboot_info* msource = (struct multiboot_info*)remap((uint64_t)mboot_addr);
    memcpy(&multiboot_info, msource, sizeof(struct multiboot_info));

    __mem_mirror_present = false;
    frame_pool = NULL;
    maxram = detect_maxram(&multiboot_info);

    create_frame_pool(&multiboot_info);
    maxphyaddr = detect_maxphyaddr();

    initialize_memory_mirror();
    __mem_mirror_present = true;
}

/**
 * Allocates frame to a page.
 *
 * Page is pointed by paddress, and attributes are derived with
 * status of kernel and readonly bools.
 *
 * If page is present, it does nothing.
 */
static puint_t allocate_frame(puint_t* paddress, bool kernel, bool readonly) {
	bool new;
	page_t page;
	memset(&page, 0, sizeof(page_t));
	if (!PRESENT(*paddress)) {
		new = true;
        proc_spinlock_lock(&__frame_lock);
        page.address = get_free_frame();
        proc_spinlock_unlock(&__frame_lock);
	} else {
		new = false;
		page.address = ALIGN(*paddress);
	}
	page.flaggable.present = 1;
	page.flaggable.rw = readonly ? 0 : 1;
	page.flaggable.us = kernel ? 0 : 1;
	*paddress = page.address;

	if (new) {
		if (__mem_mirror_present)
			memset((void*)physical_to_virtual(ALIGN(page.address)), 0xCC, 0x1000);
	}

    return ALIGN(*paddress);
}

/**
 * Deallocates frame from page, clearing the page.
 *
 * If paddress is NULL or is already free, does nothing.
 */
static void deallocate_frame(puint_t* paddress, uintptr_t va) {
    if (paddress == 0)
        return; // no upper memory structures, we are done
    if (!PRESENT(*paddress))
        return; // already cleared
    page_t page;
    page.address = *paddress;
    puint_t address = ALIGN(page.address);
    proc_spinlock_lock(&__frame_lock);
    free_frame(address);
    *paddress = 0;
    proc_spinlock_unlock(&__frame_lock);
    free_page_structure(va);
}

/**
 * Allocates memory from address from, with amount amount and with
 * provided flags.
 *
 * Repeatedly calls allocate_frame for every frame.
 */
void allocate(uintptr_t from, size_t amount, bool kernel, bool readonly) {
	amount = ALIGN_UP(amount+(from-ALIGN(from)));
	from = ALIGN(from);
    for (uintptr_t addr = from; addr < from + amount; addr += 0x1000) {
        allocate_frame(kernel ? get_page(addr, true) : get_page_user(addr, true), kernel, readonly);
    }
}

/**
 * Deallocates memory from address from with amount amount.
 *
 * Aligns the addresses to page boundaries and then
 * deallocates them all.
 */
void deallocate(uintptr_t from, size_t amount) {
    amount = ALIGN_UP(amount);
    uintptr_t aligned = from;
    if ((from % 0x1000) != 0) {
        aligned = ALIGN(from) + 0x1000;
    }
    uintptr_t end_addr = aligned + amount;
    if ((end_addr % 0x1000) != 0) {
        end_addr = ALIGN(end_addr);
    }

    if (aligned < end_addr) {
        for (uintptr_t addr = aligned; addr < end_addr; addr += 0x1000) {
            deallocate_frame(get_page(addr, false), addr);
            invalidate_address((void*)addr);
        }
    }
}

/**
 * Checks whether virtual address is allocated or not
 *
 * Uses get_page to determine the address status.
 */
bool allocated(uintptr_t addr) {
    puint_t* page = get_page(addr, false);
    if (page == NULL)
        return false;
    if (!PRESENT(*page))
        return false;
    return true;
}

puint_t clone_ptable(puint_t source_ptable, puint_t target_ptable) {
    page_table_t sptable;
    page_table_t tptable;

    if (!PRESENT(target_ptable)) {
		proc_spinlock_lock(&__frame_lock);
		target_ptable = get_free_frame();
		proc_spinlock_unlock(&__frame_lock);
	}

    sptable.number = source_ptable;
    tptable.number = target_ptable;

    uint64_t* vsptable = (uint64_t*)physical_to_virtual(ALIGN(source_ptable));
    uint64_t* vtptable = (uint64_t*)physical_to_virtual(target_ptable);
    memset(vtptable, 0, 0x1000);

    for (uint16_t i=0; i<512; i++) {
        if (PRESENT(vsptable[i])) {

            page_t page;
            page.address = vsptable[i];

            if (page.flaggable.rw == 1) {
                puint_t frame = ALIGN(page.address);
                proc_spinlock_lock(&__frame_lock);
                frame_info_t* frame_info = get_frame_info(frame);

                ++frame_info->usage_count;
                if (frame_info->cow_count == 0)
                    ++frame_info->cow_count;
                ++frame_info->cow_count;

                page_t page;
                page.address = vsptable[i];
                page.flaggable.rw = 0;
                vsptable[i] = page.address;

                proc_spinlock_unlock(&__frame_lock);
            }

            vtptable[i] = page.address;
        }
    }

    tptable.copyinfo.copy = sptable.copyinfo.copy;
    tptable.copyinfo.copy2 = sptable.copyinfo.copy2;

    return tptable.number;
}

puint_t clone_pdir(puint_t source_pdir, puint_t target_pdir) {
    page_directory_t spdir;
    page_directory_t tpdir;

    if (!PRESENT(target_pdir)) {
		proc_spinlock_lock(&__frame_lock);
		target_pdir = get_free_frame();
		proc_spinlock_unlock(&__frame_lock);
	}

    spdir.number = source_pdir;
    tpdir.number = target_pdir;

    uint64_t* vspdir = (uint64_t*)physical_to_virtual(ALIGN(source_pdir));
    uint64_t* vtpdir = (uint64_t*)physical_to_virtual(target_pdir);
    memset(vtpdir, 0, 0x1000);

    for (uint16_t i=0; i<512; i++) {
        if (PRESENT(vspdir[i])) {
            vtpdir[i] = clone_ptable(vspdir[i], vtpdir[i]);
        }
    }

    tpdir.copyinfo.copy = spdir.copyinfo.copy;
    tpdir.copyinfo.copy2 = spdir.copyinfo.copy2;

    return tpdir.number;
}


puint_t clone_pdpt(puint_t source_pdpt, puint_t target_pdpt) {
    pdpt_t spdpt;
    pdpt_t tpdpt;

    if (!PRESENT(target_pdpt)) {
		proc_spinlock_lock(&__frame_lock);
		target_pdpt = get_free_frame();
		proc_spinlock_unlock(&__frame_lock);
	}

    spdpt.number = source_pdpt;
    tpdpt.number = target_pdpt;

    uint64_t* vspdpt = (uint64_t*)physical_to_virtual(ALIGN(source_pdpt));
    uint64_t* vtpdpt = (uint64_t*)physical_to_virtual(target_pdpt);
    memset(vtpdpt, 0, 0x1000);

    for (uint16_t i=0; i<512; i++) {
        if (PRESENT(vspdpt[i])) {
            vtpdpt[i] = clone_pdir(vspdpt[i], vtpdpt[i]);
        }
    }

    tpdpt.copyinfo.copy = spdpt.copyinfo.copy;

    return tpdpt.number;
}

puint_t clone_paging_structures() {
    puint_t active_page = get_active_page();
    proc_spinlock_lock(&__frame_lock);
    puint_t target_page = get_free_frame();
    proc_spinlock_unlock(&__frame_lock);

    cr3_page_entry_t apentry;
    cr3_page_entry_t tentry;

    apentry.number = active_page;
    tentry.pml = target_page;
    tentry.copyinfo.copy = apentry.copyinfo.copy;

    uint64_t* sent = (uint64_t*) physical_to_virtual(ALIGN(active_page));
    uint64_t* tent = (uint64_t*) physical_to_virtual(target_page);
    memset(tent, 0, 0x1000);

    for (uint16_t i=0; i<512; i++) {
        if (i < 256) {
            // user pages
            if (PRESENT(sent[i])) {
                tent[i] = clone_pdpt(sent[i], tent[i]);
            }
        } else {
            // kernel pages
            tent[i] = sent[i];
        }
    }

    // flush TLB to propagate any rw changes
    broadcast_ipi_message(true, IPI_INVLD_PML, active_page, 0, NULL);
    return tentry.number;
}

// TODO: add swap?
bool page_fault(uintptr_t address, ruint_t errcode) {
    if ((errcode & (1<<1)) != 0) {
        // write error
        uint64_t* page = get_page(address, false);
        if (page != NULL) {
            ruint_t frame = ALIGN(*page);

            proc_spinlock_lock(&__frame_lock);
            frame_info_t* frame_info = get_frame_info(frame);

            if (frame_info == NULL) {
                // invalid address
                return false;
            }

            if (frame_info->cow_count == 0) {
                proc_spinlock_unlock(&__frame_lock);
                return false;
            }

            if (frame_info->cow_count == 1) {
                page_t porig;
                porig.address = *page;

                porig.flaggable.rw = 0;
                --frame_info->cow_count;

                *page = porig.address;

                proc_spinlock_unlock(&__frame_lock);
                return true;
            }

            puint_t nframe = get_free_frame();
            memcpy((void*)physical_to_virtual(nframe), (void*)physical_to_virtual(frame), 0x1000);

            page_t porig;
            page_t pnew;

            porig.address = *page;
            pnew.address = nframe;

            pnew.copyinfo.copy = porig.copyinfo.copy;
            pnew.copyinfo.copy2 = porig.copyinfo.copy2;

            pnew.flaggable.rw = 1;

            *page = pnew.address;

            --frame_info->cow_count;

            proc_spinlock_unlock(&__frame_lock);
            return true;
        }
    }

    return false;
}

void allocate_physret(uintptr_t block_addr, puint_t* physmem, bool kernel, bool rw) {
    *physmem = allocate_frame(get_page(block_addr, true), kernel, rw);
}

void mem_change_type(uintptr_t from, size_t amount,
		int change_type, bool new_value, bool invalidate_others) {
	amount = ALIGN_UP(amount);
	for (uintptr_t addr = from; addr < from + amount; addr += 0x1000) {
		puint_t* paddress = get_page(addr, true);
		if (!PRESENT(*paddress)) {
	        page_t page;
	        memset(&page, 0, sizeof(page_t));
	        page.address = *paddress;

	        if (change_type == CHNG_TYPE_RW)
	        	page.flaggable.rw = new_value;
	        if (change_type == CHNG_TYPE_SU)
	        	page.flaggable.us = new_value;
	        *paddress = page.address;
	    }
	}

	if (invalidate_others) {
		send_ipi_message(get_local_apic_id(), IPI_INVALIDATE_PAGE, from, amount, NULL);
	} else {
		broadcast_ipi_message(true, IPI_INVALIDATE_PAGE, from, amount, NULL);
	}
}
