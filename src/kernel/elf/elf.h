#pragma once

#include "../commons.h"
#include "../utils/kstdlib.h"
#include "elfdef.h"
#include "../utils/rsod.h"

typedef void  (*elf_memory_requirer_t)(void* start, size_t len);
typedef void* (*elf_memory_allocator_t)(size_t len, bool align);

typedef struct ealloc {
	elf_memory_requirer_t  requirer;
	elf_memory_allocator_t allocator;
} ealloc_t;

#define ELF_HEADER_INCORRECT 		0
#define ELF_HEADER_NO_ENTRY 		1
#define ELF_KP_EMPTY         		2
#define ELF_NOT_64	         		3
#define ELF_ENCODING_FAILURE 		4
#define ELF_VERSION_FAILURE 		5
#define ELF_NO_EXEC_EXEC_REQUIRED	6
#define ELF_RELOC_ERROR				7
#define ELF_NO_SECTIONS				8
#define ELF_ERROR_FAILED_RELOC		9
#define ELF_RELOC_UNSUP_TYPE		10


bool elf_load_kernel(Elf64_Ehdr* header, uint64_t* entry, ealloc_t alloc);
