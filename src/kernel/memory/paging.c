#include "paging.h"
#include "heap.h"

#define ALIGN(addr) (((uint64_t)addr) & 0xFFFFFFFFFFFFF000)
#define PALIGN(type, addr) ((type)ALIGN(addr))

uint64_t maxphyaddr;
uint8_t* frame_map;
uint8_t* frame_map_usage;
uint64_t maxram;
uint64_t maxframes;

extern uint64_t detect_maxphyaddr();
extern uint64_t get_active_page();
extern void set_active_page(uint64_t address);

typedef struct v_address {
	uint64_t offset :12;
	uint64_t table :9;
	uint64_t directory :9;
	uint64_t directory_ptr :9;
	uint64_t pml :9;
	uint64_t rest :16;
} v_address_t;

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

#define PRESENT(addr) (((uint64_t)addr) & 1)
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

	address = (uint64_t*) MMU_PD(vaddress)[MMU_PD_INDEX(vaddress)];

	if (!PRESENT(address)) {
		*valid = 0;
		return 0;
	}

	uint64_t physadd = MMU_PT(vaddress)[MMU_PT_INDEX(vaddress)];
	return ALIGN(physadd) + virtual_address.offset;
}


uint32_t last_searched_location;

static uint64_t get_free_frame() {
	uint32_t i, j;

	while (1) {
		for (i = 0; i < BITNSLOTS(maxframes); i++) {
			if (frame_map[i] != 0xFF) {
				// some frame available
				for (j = i * 8; j < (i + 1) * 8; j++) {
					if (!BITTEST(frame_map, j)) {
						BITSET(frame_map, j);
						last_searched_location = i;
						return j * 0x1000;
					}
				}
			}
		}

		last_searched_location = 0;
		// TODO free frames
	}

}

static uint64_t* get_page(uint64_t vaddress, bool allocate_new) {
	uint64_t* pdpt_addr = (uint64_t*) MMU_PML4(vaddress)[MMU_PML4_INDEX(
			vaddress)];

	if (!PRESENT(pdpt_addr)) {
		if (!allocate_new)
			return 0;
		pdpt_t pdpt;
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
		pt.number = get_free_frame();
		pt.flaggable.present = 1;
		MMU_PD(vaddress)[MMU_PD_INDEX(vaddress)] = pt.number;
		memset(MMU_PT(vaddress), 0, sizeof(uint64_t));
	}

	return &MMU_PT(vaddress)[MMU_PT_INDEX(vaddress)];
}

typedef struct
	__attribute__((packed)) {
		uint64_t address;
		uint64_t size;
		uint32_t type;
	} mboot_mem_t;

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
			uint32_t type = data.type;

			if (type == 0x1) {
				uint64_t totalsize = base_addr + length;
				if (totalsize > highest)
					highest = totalsize;
			}
		}
	}

	if (highest == 0) {
		error(ERROR_NO_MEMORY_DETECTED, mboot_addr->flags,
				(uint64_t) mboot_addr, &detect_maxram);
	}

	return highest;
}

static uint8_t* create_frame_map(struct multiboot* mboot_addr) {
	uint64_t i;
	uint8_t* bitmap = malign(BITNSLOTS(maxframes), 0x1);
	memset(bitmap, 0xFF, BITNSLOTS(maxframes)); // mark all ram as "used"
	frame_map_usage = malign(BITNSLOTS(maxframes), 0x1); // initial value is ignored

	if ((mboot_addr->flags & 0b1) == 1) {

		uint64_t bulk = (mboot_addr->mem_lower * 1024) / 0x1000;
		for (i = 0; i < bulk; i++) {
			BITCLEAR(bitmap, i);
		}

		bulk = (0x100000 + (mboot_addr->mem_upper * 1024)) / 0x1000;
		for (i = 0x100000 / 0x1000; i < bulk; i++) {
			BITCLEAR(bitmap, i);
		}
	}

	if ((mboot_addr->flags & 0b100000) == 0b100000) {
		uint32_t mem = mboot_addr->mmap_addr + 4;
		while (mem < (mboot_addr->mmap_addr + mboot_addr->mmap_length)) {
			mboot_mem_t data = *((mboot_mem_t*) (uint64_t) mem);
			uint32_t size = *((uint32_t*) (uint64_t) (mem - 4));
			mem += size + 4;

			uint64_t base_addr = data.address;
			uint64_t length = data.size;
			uint32_t type = data.type;

			if (type == 0x1) {
				uint64_t start_bit = base_addr / 0x1000;
				uint64_t end_bit = (base_addr + length) / 0x1000;

				for (i = start_bit; i < end_bit; i++) {
					BITCLEAR(bitmap, i);
				}
			}
		}
	}

	// mark first 2MB as used, since it is identity mapped at this point
	for (i = 0; i < 0x200000 / 0x1000; i++) {
		BITSET(bitmap, i);
	}

	return bitmap;
}


static void free_frame(uint64_t frame_address) {
	uint32_t idx = ALIGN(frame_address) / 0x1000;
	BITCLEAR(frame_map, idx);
}

void initialize_paging(struct multiboot* mboot_addr) {

	last_searched_location = 0;

	maxram = detect_maxram(mboot_addr);
	maxframes = maxram / 0x1000;

	frame_map = create_frame_map(mboot_addr);
	maxphyaddr = detect_maxphyaddr();

	for (uint32_t i = 0; i < 0x400000 / 0x1000; i++) {
		BITSET(frame_map, i);
	}
}

static void allocate_frame(uint64_t* paddress, bool kernel, bool readonly) {
	if (!PRESENT(*paddress)) {
		page_t page;
		page.address = get_free_frame();
		page.flaggable.present = 1;
		page.flaggable.rw = readonly ? 0 : 1;
		page.flaggable.us = kernel ? 0 : 1;
		*paddress = page.address;

		uint32_t idx = ALIGN(page.address) / 0x1000;
		if (kernel){
			BITCLEAR(frame_map_usage, idx);
		} else {
			BITSET(frame_map_usage, idx);
		}
	}
}

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

void allocate(uint64_t from, size_t amount, bool kernel, bool readonly) {
	for (uint64_t addr = from; addr < from + amount; addr += 0x1000) {
		allocate_frame(get_page(addr, true), kernel, readonly);
	}
}

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
		}
	}
}

bool allocated(uint64_t addr){
	uint64_t* page = get_page(addr, false);
	if (page == NULL)
		return false;
	if (!PRESENT(*page))
		return false;
	return true;
}
