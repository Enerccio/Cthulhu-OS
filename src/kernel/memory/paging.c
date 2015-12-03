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

static uint64_t get_pml(cr3_page_entry_t entry) {
	return ALIGN(entry.pml);
}

static uint64_t get_pdpt(uint64_t* table, uint16_t idx) {
	return PALIGN(uint64_t*, table)[idx];
}

static uint64_t get_page_directory(uint64_t* table, uint16_t idx) {
	return PALIGN(uint64_t*, table)[idx];
}

static uint64_t get_page_table(uint64_t* table, uint16_t idx) {
	return PALIGN(uint64_t*, table)[idx];
}

static uint64_t get_page(uint64_t* table, uint16_t idx) {
	return PALIGN(uint64_t*, table)[idx];
}

static uint64_t get_physical_offset(page_t page) {
	return ALIGN(page.address);
}

static uint64_t virtual_to_physical(uint64_t vaddress, uint8_t* valid) {
	*valid = 1;

	v_address_t virtual_address = *((v_address_t*) &vaddress);
	cr3_page_entry_t cr3_entry;
	cr3_entry.number = get_active_page();

	pml4_t pml;
	pdpt_t pdp;
	page_directory_t pagedir;
	page_table_t pagetable;
	page_t page;

	pml.number = get_pml(cr3_entry);

	if (pml.number == 0) {
		*valid = 0;
		return 0;
	}

	pdp.number = get_pdpt(pml.array, virtual_address.pml);

	if (pdp.number == 0) {
		*valid = 0;
		return 0;
	}

	pagedir.number = get_page_directory(pdp.array, virtual_address.directory_ptr);

	if (pagedir.number == 0) {
		*valid = 0;
		return 0;
	}

	pagetable.number = get_page_table(pagedir.array, virtual_address.directory);

	if (pagetable.number == 0){
		*valid = 0;
		return 0;
	}

	page.address = get_page(pagetable.array, virtual_address.table);

	if (page.address == 0) {
		*valid = 0;
		return 0;
	}

	uint64_t physadd = get_physical_offset(page);
	return physadd + virtual_address.offset;
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
	pdpt_table[511] = pml.number;

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
