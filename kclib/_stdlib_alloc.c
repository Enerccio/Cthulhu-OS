#include <stdlib.h>

#define __HEADER_VERIFIER  0xDEADD00D
#define __FOOTER_VERIFIER 0xD00DDEAD
#define __CHUNK_HEADER    0xCF
#define __MINI_HEADER     0xFC

#define __MAX_MIN_HEADER_ALLOC_SIZE (sizeof(cell_t) - sizeof(sheader_t))

typedef struct ainfo {
	uint8_t  present;
	uint8_t  starter;
	uint32_t  base_used;
	void*	 next;
	void*	 previous;

	uintptr_t start_address;
	uintptr_t total_hole;

	uintptr_t residue_address;
	uint16_t residue_count;
} ainfo_t;

#if __SIZEOF_POINTER__ == 8
typedef struct aheader {
	uint8_t padding[6];

	uint32_t magic;
	uintptr_t paddr;
	uintptr_t footer_address;
	uint32_t cnt;
	uint16_t small_magic;
} __attribute__((__packed__)) aheader_t;

typedef struct sheader {
	uint8_t padding[5];

	ainfo_t* ainfoptr;
	uint8_t  used;
	uint16_t small_magic;
} __attribute__((__packed__))  sheader_t;
#endif
#if __SIZEOF_POINTER__ == 4
typedef struct aheader {
	uint8_t padding[2];

	uint32_t magic;
	uintptr_t footer_address;
	uint32_t cnt;
	uint16_t small_magic;
} __attribute__((__packed__)) aheader_t;

typedef struct sheader {
	uint8_t padding[9];

	ainfo_t* ainfoptr;
	uint8_t  used;
	uint16_t small_magic;
} __attribute__((__packed__))  sheader_t;
#endif

typedef struct afooter {
	uint32_t magic;
	uint32_t pc;
	void* header_address;
} afooter_t;

typedef struct alloc_map {
	ainfo_t* amap;

} alloc_map_t;

typedef struct cell {
	uintptr_t padding[32];
} cell_t;


bool         __heap_initialized;
uintptr_t    __heap_address;
uintptr_t	 __highest_unused_address;
alloc_map_t  __map;

void __initialize_malloc(){
	__heap_initialized = true;
	__heap_address = (uintptr_t)__kclib_heap_start();
	__highest_unused_address = __heap_address;
}

void* __find_hole(uint32_t size, ainfo_t** valids,
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
				ptr->base_used = 0;
				*valids = ptr;
				*invalids = NULL;
				return (void*)ptr->start_address;
			} else if (ptr->residue_address != 0){
				// there is empty residue map check if it could be available
				cell_t* map = (cell_t*) ptr->start_address;
				for (uint16_t i=0; i<ptr->residue_count; i++){
					sheader_t* sh = (sheader_t*)&map[i];
					if (sh->used == 0){
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
	uintptr_t size = osize;

	if (__heap_initialized == false){
		__initialize_malloc();
	}

	if (size > __MAX_MIN_HEADER_ALLOC_SIZE){
		size = osize+sizeof(aheader_t)+sizeof(afooter_t);
	}

	ainfo_t* valid_start;
	ainfo_t** invalid_start;

	void* addr = __find_hole(size, &valid_start, &invalid_start);

	if (invalid_start != NULL){
		// we have no valid start, it means we need to allocate starting set of maps
		uint32_t rqc = (size / __ALLOC_PAGE_SIZE) + 1;
		uint32_t pgc = __ALLOC_PAGE_SIZE / sizeof(ainfo_t);
		uint32_t cpagec = pgc/rqc;

		uintptr_t address = __highest_unused_address;
		void* test = __kclib_allocate(__highest_unused_address, __ALLOC_PAGE_SIZE * rqc);
		// in kernel address  will be same as __highest_unused_address, but we can't rely on it
		__highest_unused_address += __ALLOC_PAGE_SIZE * rqc;

		if (test == 0){
			// no mem for boundaries
			return NULL;
		}

		uintptr_t nsizeb = cpagec * __ALLOC_PAGE_SIZE;
		for (uint32_t i = 0; i < cpagec; i++){
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
			ainfo->base_used = __ALLOC_PAGE_SIZE;

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
			void* test = __kclib_allocate(valid_start->start_address, __ALLOC_PAGE_SIZE);
			if (test == 0){
				// failed to allocate frame
				return NULL;
			}
			valid_start->present = 1;
		}
		uintptr_t v = (uintptr_t)addr;
		sheader_t* header = (sheader_t*)addr;
		header->used = 1;
		header->ainfoptr = valid_start;
		header->small_magic = __MINI_HEADER;
		return (void*)(v+sizeof(sheader_t));
	} else {
		// mark space
		uint32_t pguc = 0;
		ainfo_t* cadr= valid_start;
		uintptr_t byterq = size;

		while (byterq != 0){
			if (!cadr->present){
				void* test = __kclib_allocate(cadr->start_address, __ALLOC_PAGE_SIZE);
				if (test == 0){
					// failed to allocate frame
					return NULL;
				}
				cadr->present = 1;
			}
			if (byterq >= __ALLOC_PAGE_SIZE){
				cadr->residue_address = 0;
				cadr->base_used = __ALLOC_PAGE_SIZE;
				byterq -= __ALLOC_PAGE_SIZE;
			} else {
				cadr->base_used = byterq;
				cadr->residue_address = cadr->start_address + byterq;
				cadr->residue_count = ((cadr->start_address+__ALLOC_PAGE_SIZE) -
						cadr->residue_address) / sizeof(cell_t);
				memset((void*)cadr->start_address, 0, cadr->residue_count);
				byterq = 0;
			}

			cadr->total_hole = 0;
			++pguc;
		}

		uintptr_t address = valid_start->start_address;
		void* actual_addr = (void*) (address + sizeof(aheader_t));
		aheader_t* aheader = (aheader_t*)address;
		aheader->magic = __HEADER_VERIFIER;
		aheader->small_magic = __CHUNK_HEADER;
		aheader->cnt = pguc;
		aheader->footer_address = address + sizeof(aheader_t) + osize;
		aheader->paddr = (uintptr_t)valid_start;

		afooter_t* afooter = (afooter_t*)aheader->footer_address;
		afooter->header_address = aheader;
		afooter->magic = __FOOTER_VERIFIER;
		afooter->pc = pguc;

		// update old tables
		uintptr_t nsizeb = __ALLOC_PAGE_SIZE;
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

inline void* malloc(size_t s){
	return __malloc(s);
}

void __reclaim_chunks(ainfo_t* allocator){

	// check right
	ainfo_t* alright = (ainfo_t*)allocator->next;
	while (true){
		if (alright == 0)
			break;
		if (alright->present == 1){
			// to the right is used chunk
			return;
		}
		if (alright->starter == 1){
			// all chunks to the right are free up to next starter
			break;
		}
		alright = (ainfo_t*)alright->next;
	}

	// check left
	ainfo_t* alleft = allocator;
	while (true){
		if (alleft->present == 0){
			if (alleft->starter == 1){
				// this is starting chunk, free that mem page out
				__kclib_deallocate((uintptr_t)alleft, __ALLOC_PAGE_SIZE);
				if (alleft->previous != 0){
					ainfo_t* ll = (ainfo_t*)alleft->previous;
					ll->next = 0;
				} else {
					// no left chunk, null the main struct
					__map.amap = 0;
				}
				break;
			}
		} else {
			return;
		}
		alleft = (ainfo_t*)alleft->previous;
	}

	// finally, resize highest used address
	// by querying implementor if memory is unused or not
	uintptr_t new_unused = __highest_unused_address;
	for (uintptr_t ti = __highest_unused_address-__ALLOC_PAGE_SIZE ;
			ti >= __heap_address;
			ti-=__ALLOC_PAGE_SIZE){
		uint8_t reclaimed = __kclib_isreclaimed(ti, __ALLOC_PAGE_SIZE);
		if (reclaimed == 1){
			new_unused = ti;
		} else {
			__highest_unused_address = new_unused;
			return;
		}
	}
	__highest_unused_address = __heap_address;
}

void __reclaim_chunk(ainfo_t* allocator){
	if (allocator->base_used != 0)
		return; // base address used
	if (allocator->residue_count != 0){
		cell_t* map = (cell_t*) allocator->start_address;
		for (uint16_t i=0; i<allocator->residue_count; i++){
			sheader_t* sh = (sheader_t*)&map[i];
			if (sh->used == 1){
				return;
			}
		}
	}
	// no mem detected, reclaim chunk
	__kclib_deallocate(allocator->start_address, __ALLOC_PAGE_SIZE);
	allocator->present = allocator->residue_address = allocator->base_used = allocator->residue_count = 0;

	// combine right hole(s)
	allocator->total_hole = __ALLOC_PAGE_SIZE;
	if (allocator->next != 0){
		ainfo_t* alright = (ainfo_t*)allocator->next;
		if (alright->total_hole != 0){
			allocator->total_hole += alright->total_hole;
		}
	}

	// combine left hole(s)
	ainfo_t* alright = allocator->previous;
	uint64_t hole = allocator->total_hole;
	while (true){
		if (alright == NULL)
			break;

		if (alright->total_hole == 0){
			break;
		}

		alright->total_hole += hole;
		hole = alright->total_hole;

		alright = (ainfo_t*)alright->previous;
	}

	__reclaim_chunks(allocator);
}

void __free_chunk(uint32_t cdifference, uint32_t difbytes, uintptr_t alloca){
	if (cdifference == 0){
		ainfo_t* allocator = ((ainfo_t*) alloca);
		allocator->base_used -= difbytes;
		if (allocator->base_used == 0)
			__reclaim_chunk(allocator);
		return;
	}

	for (uint32_t i=0; i<cdifference; i++){
		int32_t idx = -i;
		ainfo_t* allocator = &((ainfo_t*) alloca)[idx];

		if (difbytes > __ALLOC_PAGE_SIZE){
			if (i == 0){
				uint32_t cblockdata =
						allocator->residue_address == 0 ?
								(__ALLOC_PAGE_SIZE) :
								(allocator->residue_address - allocator->start_address) ;
				difbytes -= cblockdata;
			} else {
				difbytes -= __ALLOC_PAGE_SIZE;
			}
			allocator->base_used = 0;
		} else {
			allocator->base_used -= difbytes;
			difbytes = 0;
		}

		if (allocator->base_used == 0)
			__reclaim_chunk(allocator);
	}

}

void __free(void* ptr){
	uint16_t* detect_addr = ((uint16_t*)ptr)-1;
	if (*detect_addr == __MINI_HEADER){
		sheader_t* header = ((sheader_t*)ptr)-1;
		header->used = 0;
		__reclaim_chunk(header->ainfoptr);
	} else {
		aheader_t ah = ((aheader_t*)ptr)[-1];
		__ASSERT_INTERNAL(ah.magic == __HEADER_VERIFIER);

		afooter_t af = *((afooter_t*)ah.footer_address);
		__ASSERT_INTERNAL(af.magic == __FOOTER_VERIFIER);

		__ASSERT_INTERNAL(af.pc == ah.cnt);
		__ASSERT_INTERNAL((((aheader_t*)ptr)-1) == af.header_address);

		size_t chunklen = ah.footer_address + sizeof(afooter_t) - ((uintptr_t)ptr) + sizeof(aheader_t);
		uint32_t cdifference = ah.cnt;
		uint32_t difbytes = chunklen;
		uintptr_t alloca = ah.paddr;
		__free_chunk(cdifference, difbytes, alloca);
	}
}

inline void free(void* ptr){
	__free(ptr);
}

void* calloc(size_t nmemb, size_t size){
	if (size == 0)
		return NULL;
	if (nmemb == 0)
		return NULL;

	void* data = malloc(nmemb * size);
	if (data == NULL)
		return NULL;

	memset(data, 0, nmemb * size);
	return data;
}

void* __realloc_new_chunk(void* ptr, size_t osize){
	aheader_t ah = ((aheader_t*)ptr)[-1];
	__ASSERT_INTERNAL(ah.magic == __HEADER_VERIFIER);

	afooter_t af = *((afooter_t*)ah.footer_address);
	__ASSERT_INTERNAL(af.magic == __FOOTER_VERIFIER);

	__ASSERT_INTERNAL(af.pc == ah.cnt);
	__ASSERT_INTERNAL((((aheader_t*)ptr)-1) == af.header_address);

	size_t chunklen = ah.footer_address + sizeof(afooter_t) - ((uintptr_t)ptr);
	if (osize+sizeof(afooter_t) == chunklen){
		return ptr;
	}

	if (osize < __MAX_MIN_HEADER_ALLOC_SIZE){
		void* newptr = malloc(osize);
		memmove(ptr, newptr, osize);
		free(ptr);
		return newptr;
	}

	uint32_t cdifference;
	uint32_t difbytes;
	uintptr_t alloca = ah.paddr;
	if (osize < chunklen){
		uint32_t size = osize + sizeof(afooter_t);
		uint32_t blocksize = (size / __ALLOC_PAGE_SIZE) + 1;
		cdifference = ah.cnt - blocksize;
		difbytes = chunklen - size;
	} else {
		cdifference = ah.cnt;
		difbytes = chunklen + sizeof(aheader_t);
	}

	void* newptr = ptr;
	if (osize > chunklen){
		newptr = malloc(osize);
		memmove(ptr, newptr, osize);
	}

	__free_chunk(cdifference, difbytes, alloca);

	if (osize < chunklen){
		aheader_t* a = (((aheader_t*)ptr)-1);
		a->cnt -= cdifference;
		a->footer_address = ((uintptr_t)ptr) + osize;
		afooter_t* f = (afooter_t*)a->footer_address;
		f->magic = __FOOTER_VERIFIER;
		f->header_address = a;
		f->pc = a->cnt;
	}

	return newptr;
}

void* __realloc_new_small(void* ptr, size_t osize){
	if (osize < __MAX_MIN_HEADER_ALLOC_SIZE){
		return ptr; // nothing to be done, we are lowering the amount
	} else {
		void* newptr = malloc(osize);
		memmove(ptr, newptr, __MAX_MIN_HEADER_ALLOC_SIZE);
		free(ptr);
		return newptr;
	}
}

void* __realloc(void* ptr, size_t size){
	uint16_t* detect_addr = ((uint16_t*)ptr)-1;
	if (*detect_addr == __MINI_HEADER){
		return __realloc_new_small(ptr, size);
	} else {
		return __realloc_new_chunk(ptr, size);
	}
}

inline void* realloc(void *ptr, size_t size){
	if (size == 0){
		free(ptr);
		return NULL;
	}
	return __realloc(ptr, size);
}
