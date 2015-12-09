/*
 * _alloc.c
 *
 *  Created on: Dec 9, 2015
 *      Author: enerccio
 */

#include <stdlib.h>

#define __HEADER_VERFIER  0xDEADD00D
#define __FOOTER_VERIFIER 0xD00DDEAD
#define __CHUNK_HEADER    0xCF
#define __MINI_HEADER     0xFC

#define __MAX_MIN_HEADER_ALLOC_SIZE (sizeof(cell_t) - sizeof(sheader_t))

typedef struct ainfo {
	uint8_t  present;
	uint8_t  starter;
	void*	 next;
	void*	 previous;

	uint64_t start_address;
	uint64_t total_hole;

	uint64_t residue_address;
	uint16_t residue_count;
} ainfo_t;

typedef struct aheader {
	uint32_t magic;
	uint64_t paddr;
	uint64_t footer_address;
	uint16_t cnt;
	uint16_t small_magic;
} __attribute__((__packed__)) aheader_t;

typedef struct sheader {
	uint64_t padding0;
	uint32_t padding1;
	uint8_t  padding2;

	uint8_t  free;
	uint16_t small_magic;
} __attribute__((__packed__))  sheader_t;

typedef struct afooter {
	uint32_t magic;
	uint32_t pc;
	void* header_address;
} afooter_t;

typedef struct alloc_map {
	ainfo_t* amap;

} alloc_map_t;

typedef struct cell {
	uint64_t padding[32];
} cell_t;


bool         __heap_initialized;
uint64_t     __heap_address;
uint64_t	 __highest_unused_address;
alloc_map_t  __map;

void initialize_malloc(){
	__heap_initialized = true;
	__heap_address = (uint64_t)_kclib_heap_start();
	__highest_unused_address = __heap_address;
}

void* find_hole(uint32_t size, ainfo_t** valids,
		ainfo_t*** invalids){
	if (__map.amap == NULL){
		*valids = NULL;
		*invalids = &__map.amap;
		return NULL;
	}

	ainfo_t* ptr = __map.amap;
	ainfo_t* lhptr = ptr;
	ainfo_t** lastinvalid = NULL;
	while (true) {
		if (ptr == NULL){
			*invalids = lastinvalid;
			*valids = lhptr;
			return NULL;
		}

		if (size < __MAX_MIN_HEADER_ALLOC_SIZE) {
			if (ptr->present == 0){
				// empty alloc page, use it for this small chunkhub
				ptr->residue_address = ptr->start_address;
				ptr->residue_count = __ALLOC_PAGE_SIZE / __MAX_MIN_HEADER_ALLOC_SIZE;
				ptr->total_hole = 0; // fill up chunk
				*valids = ptr;
				*invalids = NULL;
				return (void*)ptr->start_address;
			} else if (ptr->residue_address != 0){
				// there is empty residue map check if it could be available
				cell_t* map = (cell_t*) ptr->start_address;
				for (uint16_t i=0; i<ptr->residue_count; i++){
					sheader_t* sh = (sheader_t*)&map[i];
					if (sh->free == 1){
						sh->free = 0;
						*valids = ptr;
						*invalids = NULL;
						return (void*)sh;
					}
				}
			}
		} else {
			if (ptr->total_hole > size){
				// we have big enough hole from this point on, let us capitalize on it
				*valids = ptr;
				*invalids = NULL;
				return (void*)ptr->start_address;
			}
		}

		lhptr = ptr;
		lastinvalid = (ainfo_t**)&ptr->next;
		ptr = (ainfo_t*)ptr->next;
	}
}

void* __malloc(size_t osize){
	uint64_t size = osize;

	if (__heap_initialized == false){
		initialize_malloc();
	}

	if (size > __MAX_MIN_HEADER_ALLOC_SIZE){
		size = osize+sizeof(aheader_t)+sizeof(afooter_t);
	}

	ainfo_t* valid_start;
	ainfo_t** invalid_start;

	void* addr = find_hole(size, &valid_start, &invalid_start);

	if (invalid_start != NULL){
		// we have no valid start, it means we need to allocate starting set of maps
		uint16_t rqc = (size / __ALLOC_PAGE_SIZE) + 1;
		uint16_t pgc = __ALLOC_PAGE_SIZE / sizeof(ainfo_t);
		uint16_t apr = pgc/rqc;

		uint64_t address = __highest_unused_address;
		_kclib_allocate(__highest_unused_address, __ALLOC_PAGE_SIZE * apr);
		// in kernel address  will be same as __highest_unused_address, but we can't rely on it
		__highest_unused_address += __ALLOC_PAGE_SIZE * apr;
		uint16_t cpagec = apr;

		uint64_t nsizeb = cpagec * __ALLOC_PAGE_SIZE;
		for (uint16_t i = 0; i < cpagec; i++){
			ainfo_t* ainfo = (ainfo_t*)(address + (i*sizeof(ainfo_t)));
			ainfo->total_hole = (cpagec-i) * __ALLOC_PAGE_SIZE;
			ainfo->present = 0;
			ainfo->previous = 0;
			ainfo->next = (void*) ((i < (cpagec-1)) ?
					  (void*) (address + ((i+1)*sizeof(ainfo_t)))
					: 0);
			ainfo->start_address = __highest_unused_address;
			if (i == 0){
				ainfo->previous = valid_start != NULL? valid_start : 0;
			} else {
				ainfo->previous = (void*) (address + ((i-1)*sizeof(ainfo_t)));
			}
			ainfo->starter = i == 0 ? 1 : 0;
			ainfo->residue_address = 0;
			ainfo->residue_count = 0;

			__highest_unused_address += __ALLOC_PAGE_SIZE;

			if (i == 0){
				*invalid_start = ainfo;
			}
		}

		if (valid_start != NULL){
			// update the empty space before
			while (true){
				valid_start = (ainfo_t*)valid_start->previous;
				if (valid_start->present == 0 ||
						valid_start->total_hole > 0){
					valid_start->total_hole = nsizeb + __ALLOC_PAGE_SIZE;
				}
				nsizeb += __ALLOC_PAGE_SIZE;
			}
		}
	}

	if (addr == NULL){
		// we allocated enough pages right now, call malloc again when we have the pages
		return __malloc(osize);
	}

	if (size < __MAX_MIN_HEADER_ALLOC_SIZE){
		if (valid_start->present == 0){
			_kclib_allocate(valid_start->start_address, __ALLOC_PAGE_SIZE);
			valid_start->present = 1;
		}
		uint64_t v = (uint64_t)addr;
		sheader_t* header = (sheader_t*)addr;
		header->free = 0;
		header->small_magic = __MINI_HEADER;
		return (void*)(v+sizeof(sheader_t));
	} else {
		// mark space
		uint16_t pguc = 0;
		ainfo_t* cadr= valid_start;
		uint64_t byterq = size;
		while (byterq != 0){
			if (!cadr->present){
				_kclib_allocate(cadr->present, __ALLOC_PAGE_SIZE);
				cadr->present = 1;
			}
			if (byterq >= __ALLOC_PAGE_SIZE){
				cadr->residue_address = 0;
				cadr->total_hole = 0;
				byterq -= __ALLOC_PAGE_SIZE;
			} else {
				cadr->residue_address = cadr->start_address + byterq;
				cadr->total_hole = 0;
				byterq = 0;
			}
			++pguc;
		}

		uint64_t address = valid_start->start_address;
		void* actual_addr = (void*) (address + sizeof(aheader_t));
		aheader_t* aheader = (aheader_t*)address;
		aheader->magic = __HEADER_VERFIER;
		aheader->small_magic = __CHUNK_HEADER;
		aheader->cnt = pguc;
		aheader->footer_address = address + sizeof(aheader_t) + size;
		aheader->paddr = (uint64_t)valid_start;

		afooter_t* afooter = (afooter_t*)aheader->footer_address;
		afooter->header_address = aheader;
		afooter->magic = __FOOTER_VERIFIER;
		afooter->pc = pguc;

		// update old tables
		uint64_t nsizeb = __ALLOC_PAGE_SIZE;
		while (true){
			valid_start = (ainfo_t*)valid_start->previous;
			if (valid_start == NULL)
				break;
			if (valid_start->present == 0 ||
					valid_start->total_hole > 0){
				valid_start->total_hole = nsizeb + __ALLOC_PAGE_SIZE;
			}
			nsizeb += __ALLOC_PAGE_SIZE;
		}

		return actual_addr;
	}
}

void* malloc(size_t s){
	return __malloc(s);
}
