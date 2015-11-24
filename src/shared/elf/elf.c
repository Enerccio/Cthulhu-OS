#include "elf.h"

void elf_load_kernel(Elf64_Ehdr* header)
{
	if (header->e_ident[0] != ELFMAG0 ||
		header->e_ident[1] != ELFMAG1 ||
		header->e_ident[2] != ELFMAG2 ||
		header->e_ident[3] != ELFMAG3 ){
		uint32_t elfmagic = 0;
		elfmagic += ELFMAG0 << 24;
		elfmagic += ELFMAG1 << 16;
		elfmagic += ELFMAG2 << 8;
		elfmagic += ELFMAG3;

		uint32_t actmagic = 0;
		actmagic += header->e_ident[0] << 24;
		actmagic += header->e_ident[1] << 16;
		actmagic += header->e_ident[2] << 8;
		actmagic += header->e_ident[3];
		error(LOADER_KERNEL_UNAVAILABLE, actmagic, elfmagic, ELF_HEADER_INCORRECT);
	}

    if (header->e_entry == 0)
    	error(LOADER_KERNEL_UNAVAILABLE, 0, 0, ELF_HEADER_NOENTRIES);

    uint32_t program_code_size = header->e_phentsize * header->e_phnum;

    if (program_code_size == 0)
    	error(LOADER_KERNEL_UNAVAILABLE, 0, 0, ELF_KP_EMPTY);

}
