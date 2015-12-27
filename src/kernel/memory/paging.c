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

/** aligns the address to 0x1000 */
#define ALIGN(addr) (((uint64_t)addr) & 0xFFFFFFFFFFFFF000)
/** aligns the address to 0x1000 and then casts it to type */
#define PALIGN(type, addr) ((type)ALIGN(addr))

/** Maximum physical bits in page structures */
uint64_t maxphyaddr;
/** Maximum ram address, might be unusable */
uint64_t maxram;

section_info_t* frame_pool;

extern uint64_t detect_maxphyaddr();
extern uint64_t get_active_page();
extern void set_active_page(uint64_t address);
extern void invalidate_address(uint64_t address);
extern uint64_t is_1GB_paging_supported();

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
uint64_t virtual_to_physical(uint64_t vaddress, uint8_t* valid) {
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
                vaddress < ADDRESS_OFFSET((RESERVED_KBLOCK_RAM_MAPPINGS+1))	&&
				is_1GB_paging_supported()){
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

    uint64_t physadd = MMU_PT(vaddress)[MMU_PT_INDEX(vaddress)];
    return ALIGN(physadd) + virtual_address.offset;
}

/**
 * Returns address + physical identity map offset
 */
uint64_t physical_to_virtual(uint64_t vaddress) {
    return vaddress + ADDRESS_OFFSET(RESERVED_KBLOCK_RAM_MAPPINGS);
}

/** contains last found free frame, so algorithm can continue from that point when searching for next */
uint32_t last_searched_location;

/**
 * Always returns free frame
 * TODO: add paging to swap
 */
extern void kp_halt();
static uint64_t get_free_frame() {
	if (frame_pool == NULL){
		return ((uint64_t)malign(0x1000, 0x1000)-0xFFFFFFFF80000000);
	} else {
		// TODO add synchronization
		section_info_t* section = frame_pool;
		while (section != NULL){
			if (section->head != NULL){
				stack_element_t* se = section->head;
				section->head = se->next;
				section->frame_array[se->array_ord].usage_count = 0;
				return se->frame_address;
			}
			section = section->next_section;
		}

		// TODO handle crap here
		kp_halt();
		return 0;
	}
}

/**
 * Deallocates frame from frame_map.
 */
static void free_frame(uint64_t frame_address) {
    uint64_t fa = ALIGN(frame_address);
	// TODO add synchronization
	section_info_t* section = frame_pool;
	while (section != NULL){
		if (section->start_word >= fa && section->end_word < fa){
			uint32_t idx = (fa-section->start_word) / 0x1000;
			--section->frame_array[idx].usage_count;
			if (section->frame_array[idx].usage_count == 0){
				section->frame_array[idx].bound_stack_element->next = section->head;
				section->head = section->frame_array[idx].bound_stack_element;
				memset((void*)physical_to_virtual(section->head->frame_address), 0xDE, 0x1000);
			}
		} else {
			section = section->next_section;
		}
	}

	// Should never happen
	// TODO add error
	kp_halt();
}

/**
 * Returns page allocated for that virtual address.
 *
 * If allocate_new is specified to true, it will allocate substructures,
 * if not present.Otherwise returns NULL if page structures are not present
 * on the way to the virtual address. Returned pointer points to page in page
 * table.
 */
static uint64_t* get_page(uint64_t vaddress, bool allocate_new) {
    uint64_t* pdpt_addr = (uint64_t*) MMU_PML4(vaddress)[MMU_PML4_INDEX(
            vaddress)];

    if (!PRESENT(pdpt_addr)) {
        if (!allocate_new)
            return 0;
        pdpt_t pdpt;
        memset(&pdpt, 0, sizeof(pdpt_t));
        pdpt.number = get_free_frame();
        pdpt.flaggable.present = 1;
        MMU_PML4(vaddress)[MMU_PML4_INDEX(vaddress)] = pdpt.number;
        memset(MMU_PDPT(vaddress), 0, sizeof(uint64_t));
    }

    uint64_t* pdir_addr = (uint64_t*) MMU_PDPT(vaddress)[MMU_PDPT_INDEX(
            vaddress)];

    if (!PRESENT(pdir_addr)) {
        if (!allocate_new)
            return 0;
        page_directory_t dir;
        memset(&dir, 0, sizeof(page_directory_t));
        dir.number = get_free_frame();
        dir.flaggable.present = 1;
        MMU_PDPT(vaddress)[MMU_PDPT_INDEX(vaddress)] = dir.number;
        memset(MMU_PD(vaddress), 0, sizeof(uint64_t));
    }

    uint64_t* pt_addr = (uint64_t*) MMU_PD(vaddress)[MMU_PD_INDEX(vaddress)];

    if (!PRESENT(pt_addr)) {
        if (!allocate_new)
            return 0;
        page_table_t pt;
        memset(&pt, 0, sizeof(page_table_t));
        pt.number = get_free_frame();
        pt.flaggable.present = 1;
        MMU_PD(vaddress)[MMU_PD_INDEX(vaddress)] = pt.number;
        memset(MMU_PT(vaddress), 0, sizeof(uint64_t)*512);
    }

    return &MMU_PT(vaddress)[MMU_PT_INDEX(vaddress)];
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
static uint64_t detect_maxram(struct multiboot* mboot_addr) {
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
        error(ERROR_NO_MEMORY_DETECTED, mboot_addr->flags,
                (uint64_t) mboot_addr, &detect_maxram);
    }

    return highest;
}

/**
 * Creates frame map from multiboot structure.
 *
 * Fills bitmaps for frame_map_usage, phys_map and returns bitmap for frame_map
 * from available ram entries.
 */
static void create_frame_pool(struct multiboot* mboot_addr) {
	section_info_t* firstfp = NULL;
	section_info_t* lastfp = NULL;

	if ((mboot_addr->flags & 0b100000) == 0b100000) {
		uint32_t mem = mboot_addr->mmap_addr + 4;
		while (mem < (mboot_addr->mmap_addr + mboot_addr->mmap_length)) {
			mboot_mem_t data = *((mboot_mem_t*) (uint64_t) mem);
			uint32_t size = *((uint32_t*) (uint64_t) (mem - 4));
			mem += size + 4;

			uint64_t base_addr = data.address;
			uint64_t length = data.size;

			if (data.type != 1) // not a ram
				continue;

			if (base_addr+length < 0x400000) // ram is below autoallocated amount
				continue;

			if (base_addr < 0x400000){ // ram starts below 0x400000 but grows past it
				length = length - (0x400000-base_addr);
				base_addr = 0x400000;
			}

			if (length < 0x10000) // ram is effectively too small to use
				continue;

			// total amount of frames available if we count in metadata
			uint64_t uframes = (length-sizeof(section_info_t))/(0x1000+(sizeof(stack_element_t)+sizeof(frame_info_t)))-1;
			// total size of metadata header
			uint64_t total_size = sizeof(section_info_t) + (uframes * sizeof(stack_element_t)) + (uframes * sizeof(frame_info_t));
			// address of first effective frame
			uint64_t after_address = (base_addr + total_size + 0x1000) & ~0xFFF;

			// allocate base_addr and size of metadata header in virt. memory
			allocate(base_addr, total_size, true, false);

			// section starts at base address
			section_info_t* section = (section_info_t*)base_addr;
			if (lastfp != NULL){ // link previous section and this one
				lastfp->next_section = section;
			} else if (firstfp == NULL){ // first section will be this one
				firstfp = section;
			}

			// fill up section with information
			section->start_word = after_address;
			section->end_word = base_addr + length;
			section->total_frames = uframes;
			section->next_section = NULL;
			section->frame_array = (frame_info_t*) (base_addr + sizeof(section_info_t)); // starts at end of this structure
			uint64_t stack_el_addr = (base_addr + sizeof(section_info_t) +
					(sizeof(frame_info_t) * uframes)); // first stack element describing the memory
			section->head = (stack_element_t*) stack_el_addr; // set it as head

			stack_element_t* prev_se = NULL;
			for (uint64_t i=0; i<uframes; i++){
				// Fill up information for stack element.
				// Stack element contains frame address of free frame in this memory section
				stack_element_t* se = (stack_element_t*)stack_el_addr;
				stack_el_addr += sizeof(stack_element_t);
				se->frame_address = after_address + (i*0x1000);
				// link stack elements together
				if (prev_se != NULL)
					prev_se->next = se;
				se->next = NULL;
				se->array_ord = i;
				prev_se = se;

				// fill up frame info with this stack element and number of times it has been used (0)
				frame_info_t* fi = &section->frame_array[i];
				fi->usage_count = 0;
				fi->bound_stack_element = se;
			}

			lastfp = section;
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
    if (is_1GB_paging_supported() != 0){
        log_msg("Host supports 1GB pages, will use for ram mirror");
        for (uint64_t start=0; start < maxram; start+=1<<30){
            uint64_t vaddress = start + ADDRESS_OFFSET(RESERVED_KBLOCK_RAM_MAPPINGS);
            uint64_t* pdpt_addr = (uint64_t*) MMU_PML4(vaddress)[MMU_PML4_INDEX(
                        vaddress)];

            if (!PRESENT(pdpt_addr)) {
                pdpt_t pdpt;
                pdpt.number = get_free_frame();
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
        log_warn("Host does not provide 1GB pages, will use 4KB pages for ram mirror");
        for (uint64_t start=0; start < maxram; start+=0x1000){
            uint64_t vaddress = start + ADDRESS_OFFSET(RESERVED_KBLOCK_RAM_MAPPINGS);
            uint64_t* paddress = get_page(vaddress, true);
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
 *  2. denotes max frames from that
 *  3. creates frame_map via create_frame_map
 *  4. detects max physical address bits
 *  5. marks first 4MBytes of physical address as used
 *  6. initializes memory mirror
 */
void initialize_paging(struct multiboot* mboot_addr) {
    frame_pool = NULL;
    maxram = detect_maxram(mboot_addr);

    create_frame_pool(mboot_addr);
    maxphyaddr = detect_maxphyaddr();

    initialize_memory_mirror();
}

/**
 * Allocates frame to a page.
 *
 * Page is pointed by paddress, and attributes are derived with
 * status of kernel and readonly bools.
 *
 * If page is present, it does nothing.
 */
static void allocate_frame(uint64_t* paddress, bool kernel, bool readonly) {
    if (!PRESENT(*paddress)) {
        page_t page;
        memset(&page, 0, sizeof(page_t));
        page.address = get_free_frame();
        page.flaggable.present = 1;
        page.flaggable.rw = readonly ? 0 : 1;
        page.flaggable.us = kernel ? 0 : 1;
        *paddress = page.address;
    }
}

/**
 * Deallocates frame from page, clearing the page.
 *
 * If paddress is NULL or is already free, does nothing.
 */
static void deallocate_frame(uint64_t* paddress) {
    if (paddress == 0)
        return; // no upper memory structures, we are done
    if (!PRESENT(*paddress))
        return; // already cleared
    page_t page;
    page.address = *paddress;
    memset(paddress, 0, sizeof(uint64_t));
    uint64_t address = ALIGN(page.address);
    free_frame(address);
}

/**
 * Allocates memory from address from, with amount amount and with
 * provided flags.
 *
 * Repeatedly calls allocate_frame for every frame.
 */
void allocate(uint64_t from, size_t amount, bool kernel, bool readonly) {
    for (uint64_t addr = from; addr < from + amount; addr += 0x1000) {
        allocate_frame(get_page(addr, true), kernel, readonly);
    }
}

/**
 * Deallocates memory from address from with amount amount.
 *
 * Aligns the addresses to page boundaries and then
 * deallocates them all.
 */
void deallocate(uint64_t from, size_t amount) {
    uint64_t aligned = from;
    if ((from % 0x1000) != 0) {
        aligned = ALIGN(from) + 0x1000;
    }
    uint64_t end_addr = aligned + amount;
    if ((end_addr % 0x1000) != 0) {
        end_addr = ALIGN(end_addr);
    }

    if (aligned < end_addr) {
        for (uint64_t addr = aligned; addr < end_addr; addr += 0x1000) {
            memset((void*)addr, 0, 0x1000);
            deallocate_frame(get_page(addr, false));
            invalidate_address(addr);
        }
    }
}

/**
 * Checks whether virtual address is allocated or not
 *
 * Uses get_page to determine the address status.
 */
bool allocated(uint64_t addr) {
    uint64_t* page = get_page(addr, false);
    if (page == NULL)
        return false;
    if (!PRESENT(*page))
        return false;
    return true;
}
