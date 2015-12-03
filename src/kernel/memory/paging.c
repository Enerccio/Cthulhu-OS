#include "paging.h"
#include "heap.h"

#define ALIGN(addr) (((uint64_t)addr) & 0xFFFFFFFFFFFFF000)
#define PALIGN(type, addr) ((type)ALIGN(addr))

uint64_t maxphyaddr;
uint8_t* frame_map;
uint64_t maxram;
uint64_t maxframes;

extern uint64_t detect_maxphyaddr();
extern uint64_t get_active_page();
extern void		set_active_page(uint64_t address);

typedef struct v_address {
	uint64_t offset :12;
	uint64_t table :9;
	uint64_t directory :9;
	uint64_t directory_ptr :9;
	uint64_t pml :9;
	uint64_t rest :16;
} v_address_t;

#define MMU_RECURSIVE_SLOT      (510UL)

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

	uint64_t* address = MMU_PML4(vaddress);
	address = address[MMU_PML4_INDEX(vaddress)];

	if (!PRESENT(address)) {
		*valid = 0;
		return 0;
	}

	address = MMU_PDPT(vaddress)[MMU_PDPT_INDEX(vaddress)];

	if (!PRESENT(address)) {
		*valid = 0;
		return 0;
	}

	address = MMU_PD(vaddress)[MMU_PD_INDEX(vaddress)];

	if (!PRESENT(address)) {
		*valid = 0;
		return 0;
	}

	uint64_t physadd = MMU_PT(vaddress)[MMU_PT_INDEX(vaddress)];
	return ALIGN(physadd) + virtual_address.offset;
}

static uint64_t* get_page(uint64_t vaddress){
	uint64_t* plm_addr = MMU_PML4(vaddress);
	uint64_t* pdpt_addr = plm_addr[MMU_PML4_INDEX(vaddress)];

	if (!PRESENT(pdpt_addr)){
		pdpt_t pdpt;
		uint64_t* pdir_table = (uint64_t*)malign_p(sizeof(uint64_t)*512, 0x1000);
		memset(pdir_table, 0, sizeof(uint64_t)*512);
		pdpt.array = (uint64_t*)virtual_to_physical((uint64_t)pdir_table, 0);
		pdpt.flaggable.present = 1;
		pdpt_addr = (uint64_t*)plm_addr[MMU_PML4_INDEX(vaddress)];
		*pdpt_addr = pdpt.number;
	}

	uint64_t* pdir_addr = MMU_PDPT(vaddress);
	pdir_addr = pdir_addr[MMU_PDPT_INDEX(vaddress)];

	if (!PRESENT(pdir_addr)){
		page_directory_t dir;
		uint64_t* p_table = (uint64_t*)malign_p(sizeof(uint64_t)*512, 0x1000);
		memset(p_table, 0, sizeof(uint64_t)*512);
		dir.array = (uint64_t*)virtual_to_physical((uint64_t)p_table, 0);
		dir.flaggable.present = 1;
		MMU_PDPT(vaddress)[MMU_PDPT_INDEX(vaddress)] = dir.number;
	}

	uint64_t* pt_addr = MMU_PD(vaddress);
	pt_addr = pt_addr[MMU_PD_INDEX(vaddress)];

	if (!PRESENT(pt_addr)){
		page_table_t pt;
		page_t* pages = (page_t*)malign_p(sizeof(page_t)*512, 0x1000);
		memset(pages, 0, sizeof(page_t)*512);
		pt.array = (uint64_t*)virtual_to_physical((uint64_t)pages, 0);
		pt.flaggable.present = 1;
		MMU_PD(vaddress)[MMU_PD_INDEX(vaddress)] = pt.number;
	}

	uint64_t* pt = MMU_PT(vaddress);
	return &pt[MMU_PT_INDEX(vaddress)];
}

typedef struct
	__attribute__((packed)){
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
		printf("... Detecting extended ram segments:\n");
		uint32_t mem = mboot_addr->mmap_addr + 4;
		while (mem < (mboot_addr->mmap_addr + mboot_addr->mmap_length)) {
			mboot_mem_t data = *((mboot_mem_t*) (uint64_t) mem);
			uint32_t size = *((uint32_t*) (uint64_t) (mem - 4));
			mem += size + 4;

			uint64_t base_addr = data.address;
			uint64_t length = data.size;
			uint32_t type = data.type;

			printf(
					"... ... Segment at address 0x%lX, size %ld, type 0x%X\n",
					base_addr, length, type);

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
	uint8_t* bitmap = malloc(BITNSLOTS(maxframes));
	memset(bitmap, 0xFF, BITNSLOTS(maxframes)); // mark all ram as "used"

	if ((mboot_addr->flags & 0b1) == 1) {

		uint64_t bulk = (mboot_addr->mem_lower*1024)/0x1000;
		for (i=0; i<bulk; i++){
			BITCLEAR(bitmap, i);
		}

		bulk = (0x100000+(mboot_addr->mem_upper*1024))/0x1000;
		for (i=0x100000/0x1000; i<bulk; i++){
			BITCLEAR(bitmap, i);
		}
	}

	if ((mboot_addr->flags & 0b100000) == 0b100000) {
		printf("... Detecting extended ram segments:\n");
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

				for (i=start_bit; i<end_bit; i++){
					BITCLEAR(bitmap, i);
				}
			}
		}
	}

	// mark first 2MB as used, since it is identity mapped at this point
	for (i=0; i<0x200000/0x1000; i++){
		BITSET(bitmap, i);
	}

	return bitmap;
}

uint32_t last_searched_location;

static uint64_t get_free_frame(){
	uint32_t i, j;

	while (1) {
		for (i=0; i<BITNSLOTS(maxframes); i++){
			if (frame_map[i] != 0xFF){
				// some frame available
				for (j=i*8; j<(i+1)*8; j++){
					if (!BITTEST(frame_map, j)){
						BITSET(frame_map, j);
						last_searched_location = i;
						return j*0x1000;
					}
				}
			}
		}

		last_searched_location = 0;
		// TODO free frames
	}

}

static void free_frame(uint64_t frame_address){
	uint32_t idx = ALIGN(frame_address)/0x1000;
	BITCLEAR(frame_map, idx);
}

static void create_initial_paging(){
	// identity map 4MB, therefore use first frames for that
	for (uint32_t i=0; i<0x400000/0x1000; i++){
		BITSET(frame_map, i);
	}

	pml4_t pml;
	uint64_t* pdpt_table = (uint64_t*)malign(sizeof(uint64_t)*512, 0x1000);
	memset(pdpt_table, 0, sizeof(uint64_t)*512);
	pml.array = pdpt_table;
	pml.flaggable.present = 1;

	pdpt_t pdpt;
	uint64_t* pdir_table = (uint64_t*)malign(sizeof(uint64_t)*512, 0x1000);
	memset(pdir_table, 0, sizeof(uint64_t)*512);
	pdpt.array = pdir_table;
	pdpt.flaggable.present = 1;
	pdpt_table[0] = pdpt.number;

	page_directory_t dir;
	uint64_t* p_table = (uint64_t*)malign(sizeof(uint64_t)*512, 0x1000);
	memset(p_table, 0, sizeof(uint64_t)*512);
	dir.array = p_table;
	dir.flaggable.present = 1;
	pdir_table[0] = dir.number;

	page_table_t pt;
	page_t* pages = (page_t*)malign(sizeof(page_t)*512, 0x1000);
	memset(pages, 0, sizeof(page_t)*512);
	pt.array = (uint64_t*)pages;
	pt.flaggable.present = 1;
	p_table[0] = pt.number;

	for (uint32_t j=0; j<512; j++){
		page_t page = pages[j];
		page.address = j*0x1000;
		page.flaggable.present = 1;
		page.flaggable.rw = 1;

		pages[j] = page;
	}

	pages = (page_t*)malign(sizeof(page_t)*512, 0x1000);
	memset(pages, 0, sizeof(page_t)*512);
	pt.array = (uint64_t*)pages;
	pt.flaggable.present = 1;
	p_table[1] = pt.number;

	for (uint32_t j=0; j<512; j++){
		page_t page = pages[j];
		page.address = j*0x1000 + (512*0x1000);
		page.flaggable.present = 1;
		page.flaggable.rw = 1;

		pages[j] = page;
	}

	// fractal it
	pdpt_table[MMU_RECURSIVE_SLOT] = pml.number;

	cr3_page_entry_t entry;
	entry.pml = pml.number;

	set_active_page(entry.number);
}

void initialize_paging(struct multiboot* mboot_addr) {
	printf("Initializing paging memory... \n");

	last_searched_location = 0;

	maxram = detect_maxram(mboot_addr);
	maxframes = maxram / 0x1000;

	printf("... Highest memory address: 0%lXd \n", maxram);
	printf("... Number of physical frames available: %ld \n", maxframes);

	frame_map = create_frame_map(mboot_addr);
	maxphyaddr = detect_maxphyaddr();

	printf("... Max physical address bits: %ld \n", maxphyaddr);

	create_initial_paging();
}

void allocate_frame(uint64_t* paddress, bool kernel, bool readonly)
{
	if (!PRESENT(*paddress)){
		page_t page;
		page.address = get_free_frame();
		page.flaggable.present = 1;
		page.flaggable.rw = readonly ? 0 : 1;
		page.flaggable.us = kernel ? 0 : 1;
		*paddress = page.address;
	}
}

void allocate(uint64_t from, size_t amount, bool kernel, bool readonly)
{
	for (uint64_t addr = from; addr < from+amount; addr += 0x1000)
	{
		allocate_frame(get_page(addr), kernel, readonly);
	}
}


