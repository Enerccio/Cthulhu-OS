#include "elf.h"

#ifdef KERNEL64BIT
#define INTCAST(V)  V
#define UINTCAST(V) V
#define PTR      	uint64_t
#define SPTR     	int64_t
#define PTRCAST(V)  ((PTR)(V))
#else
#define INTCAST(V)  ((int32_t)(V))
#define UINTCAST(V) ((uint32_t)(V))
#define PTR      	uint32_t
#define SPTR     	int32_t
#define PTRCAST(V)  ((PTR)(V))
#endif

#define ELF_ERROR(A1, A2) \
	do { if (rsod) error(LOADER_KERNEL_UNAVAILABLE, A1, A2, header); else return false; } while (0)

static const char* elf_lookup_symbol(const char* addr __attribute__((unused)), Elf64_Ehdr* elf __attribute__((unused))) {
	return NULL;
}

static bool elf_header_check(Elf64_Ehdr* header, bool rsod) {
	if (header->e_ident[EI_MAG0] != ELFMAG0
			|| header->e_ident[EI_MAG1] != ELFMAG1
			|| header->e_ident[EI_MAG2] != ELFMAG2
			|| header->e_ident[EI_MAG3] != ELFMAG3) {

		uint64_t actmagic = 0;
		actmagic += header->e_ident[EI_MAG0] << 24;
		actmagic += header->e_ident[EI_MAG1] << 16;
		actmagic += header->e_ident[EI_MAG2] << 8;
		actmagic += header->e_ident[EI_MAG3];
		ELF_ERROR(actmagic, ELF_HEADER_INCORRECT);
	}

	if (header->e_ident[EI_CLASS] != ELFCLASS64)
		ELF_ERROR(header->e_ident[EI_CLASS], ELF_NOT_64);

	if (header->e_ident[EI_DATA] == ELFDATANONE)
		ELF_ERROR(header->e_ident[EI_DATA], ELF_ENCODING_FAILURE);

	if (header->e_ident[EI_VERSION] != EV_CURRENT)
		ELF_ERROR(header->e_ident[EI_VERSION], ELF_VERSION_FAILURE);

	if (header->e_version != EV_CURRENT)
		ELF_ERROR(header->e_version, ELF_VERSION_FAILURE);

	return true;
}

static inline Elf64_Shdr* elf_sheader(Elf64_Ehdr* hdr) {
	return (Elf64_Shdr*) UINTCAST(((int64_t) PTRCAST(hdr)) + hdr->e_shoff);
}

static inline Elf64_Shdr* elf_section(Elf64_Ehdr* hdr, int64_t idx) {
	return &elf_sheader(hdr)[idx];
}

static inline char* elf_str_table(Elf64_Ehdr* hdr) {
	if (hdr->e_shstrndx == SHN_UNDEF)
		return NULL;
	return (char *) hdr + elf_section(hdr, hdr->e_shstrndx)->sh_offset;
}

static inline char* elf_lookup_string(Elf64_Ehdr* hdr, int64_t offset) {
	char* strtab = elf_str_table(hdr);
	if (strtab == NULL)
		return NULL;
	return strtab + offset;
}

static bool elf_get_symval(Elf64_Ehdr *hdr, int64_t table, uint64_t idx,
		uint64_t* address) {
	if (table == SHN_UNDEF || idx == SHN_UNDEF)
		return false;

	Elf64_Shdr *symtab = elf_section(hdr, table);

	uint64_t symtab_entries = symtab->sh_size / symtab->sh_entsize;
	if (idx >= symtab_entries) {
		return false;
	}

	int64_t symaddr = ((int64_t) PTRCAST(hdr)) + symtab->sh_offset;
	Elf64_Sym* symbol = &((Elf64_Sym*) PTRCAST(symaddr))[idx];
	if (symbol->st_shndx == SHN_UNDEF) {
		// External symbol, lookup value
		Elf64_Shdr* strtab = elf_section(hdr, symtab->sh_link);
		const char* name = (const char*) hdr + strtab->sh_offset
				+ symbol->st_name;

		void* target = (void*) elf_lookup_symbol(name, hdr);

		if (target == NULL) {
			// Extern symbol not found
			if (ELF64_ST_BIND(symbol->st_info) & STB_WEAK) {
				*address = 0;
				return true;
			} else {
				return false;
			}
		} else {
			*address = (uint64_t) PTRCAST(target);
			return true;
		}
	} else if (symbol->st_shndx == SHN_ABS) {
		// Absolute symbol
		*address = (uint64_t) symbol->st_value;
		return true;
	} else {
		// Internally defined symbol
		Elf64_Shdr* target = elf_section(hdr, symbol->st_shndx);
		*address = (((uint64_t) PTRCAST(hdr)) + symbol->st_value + target->sh_offset);
		return true;
	}
}

static bool elf_load_stage1(Elf64_Ehdr* header, ealloc_t alloc) {
	Elf64_Shdr *shdr = elf_sheader(header);

	unsigned int i;
	// Iterate over section headers
	for (i = 0; i < header->e_shnum; i++) {
		Elf64_Shdr *section = &shdr[i];

		// If the section isn't present in the file
		if (section->sh_type == SHT_NOBITS) {
			// Skip if it the section is empty
			if (!section->sh_size)
				continue;

			// If the section should appear in memory
			if (section->sh_flags & SHF_ALLOC) {
				// Allocate and zero some memory
				void *mem = alloc.allocator(section->sh_size, false);
				memset(mem, 0, section->sh_size);

				// Assign the memory offset to the section offset
				section->sh_offset = (uint64_t) (PTRCAST(mem)
						- PTRCAST(header));
			}
		}
	}
	return true;
}

static int elf_do_reloc(Elf64_Ehdr *header, Elf64_Rel *rel, Elf64_Shdr *reltab, bool rsod) {
	Elf64_Shdr *target = elf_section(header, reltab->sh_info);

	int64_t addr = (int64_t) ((uint64_t)PTRCAST(header)) + target->sh_offset;
	int64_t *ref = (int64_t*) PTRCAST(addr + rel->r_offset);
	// Symbol value
	uint64_t symval = 0;
	if (ELF64_R_SYM(rel->r_info) != SHN_UNDEF) {
		if (elf_get_symval(header, reltab->sh_link, ELF64_R_SYM(rel->r_info), &symval) == false)
			return false;
	}
	// Relocate based on type
	switch (ELF64_R_TYPE(rel->r_info)) {
	case R_X86_64_NONE:
		// No relocation
		break;
	case R_X86_64_64:
		// Symbol + Offset
		*ref = DO_X86_64_64(symval, *ref);
		break;
	case R_X86_64_PC32:
		// Symbol + Offset - Section Offset
		*ref = DO_X86_64_PC32(symval, *ref, (int64_t) INTCAST(ref));
		break;
	default:
		ELF_ERROR(ELF64_R_TYPE(rel->r_info), ELF_RELOC_UNSUP_TYPE);
	}
	return symval;
}

static bool elf_load_stage2(Elf64_Ehdr* header, bool rsod) {
	Elf64_Shdr *shdr = elf_sheader(header);

	uint32_t i;
	PTR idx;
	// Iterate over section headers
	for (i = 0; i < header->e_shnum; i++) {
		Elf64_Shdr *section = &shdr[i];

		// If this is a relocation section
		if (section->sh_type == SHT_REL) {
			// Process each entry in the table
			for (idx = 0; idx < section->sh_size / section->sh_entsize; idx++) {
				Elf64_Rel *reltab = &((Elf64_Rel*) PTRCAST(UINTCAST(header)
						+ UINTCAST(section->sh_offset)))[idx];
				bool result = elf_do_reloc(header, reltab, section, rsod);
				// On error, display a message and return
				if (result == false)
					ELF_ERROR((uint64_t) UINTCAST(reltab), ELF_ERROR_FAILED_RELOC);
			}
		}
	}
	return true;
}

static bool elf_load_kernel_exec(Elf64_Ehdr* header, ealloc_t alloc) {
	static bool rsod = true;

	return true;
}

static bool elf_load_kernel_rel(Elf64_Ehdr* header, ealloc_t alloc) {
	static bool rsod = true;
	bool result;

	result = elf_load_stage1(header, alloc);
	if (result == false)
		ELF_ERROR(0, ELF_RELOC_ERROR);

	result = elf_load_stage2(header, rsod);
	if (result == false)
		ELF_ERROR(0, ELF_RELOC_ERROR);

	return true;
}

bool elf_load_kernel(Elf64_Ehdr* header, uint64_t* entry, ealloc_t alloc) {
	static bool rsod = true;

	elf_header_check(header, true);

	if (header->e_entry == 0)
		ELF_ERROR(0, ELF_HEADER_NO_ENTRY);

	*entry = header->e_entry;

	if (header->e_type != ET_EXEC && header->e_type != ET_REL)
		ELF_ERROR(header->e_type, ELF_NO_EXEC_EXEC_REQUIRED);

	uint32_t program_code_size = header->e_phentsize * header->e_phnum;

	if (program_code_size == 0)
		ELF_ERROR(0, ELF_KP_EMPTY);

	if (header->e_shentsize * header->e_shnum == 0)
		ELF_ERROR(0, ELF_NO_SECTIONS);

	if (header->e_type == ET_EXEC)
		return elf_load_kernel_exec(header, alloc);
	else
		return elf_load_kernel_rel(header, alloc);
}
