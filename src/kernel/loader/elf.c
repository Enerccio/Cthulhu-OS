/*
 * The MIT License (MIT)
 * Copyright (c) 2015 Peter Vanusanik <admin@en-circle.com>
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy 
 * of this software and associated documentation files (the "Software"), to deal in 
 * the Software without restriction, including without limitation the rights to use, copy, 
 * modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, 
 * and to permit persons to whom the Software is furnished to do so, subject to the 
 * following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all copies 
 * or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, 
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, 
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS 
 * OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, 
 * WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN 
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 *
 * elf.c
 *  Created on: Jan 4, 2016
 *      Author: Peter Vanusanik
 *  Contents: 
 */

#include "elf.h"
#include "../memory/paging.h"
#include "../cpus/ipi.h"

extern uintptr_t get_active_page();

int32_t check_elf_header(Elf64_Ehdr* header) {
    if (header->e_ident[EI_MAG0] != ELFMAG0) {
        return ELF_ERROR_HEADER_MAGIC_INCORRECT;
    }

    if (header->e_ident[EI_MAG1] != ELFMAG1) {
        return ELF_ERROR_HEADER_MAGIC_INCORRECT;
    }

    if (header->e_ident[EI_MAG2] != ELFMAG2) {
        return ELF_ERROR_HEADER_MAGIC_INCORRECT;
    }

    if (header->e_ident[EI_MAG3] != ELFMAG3) {
        return ELF_ERROR_HEADER_MAGIC_INCORRECT;
    }

    if (header->e_ident[EI_CLASS] != ELFCLASS64) {
        return ELF_ERROR_NOTCLASS64;
    }

    if (header->e_ident[EI_DATA] != ELFDATA2LSB) {
        return ELF_ERROR_NOTDATA2LSB;
    }

    if (header->e_machine != EM_X86_64) {
        return ELF_ERROR_MACHINE_INVALID;
    }

    if (header->e_version != EV_CURRENT) {
        return ELF_ERROR_VERSION_NOT_CURRENT;
    }

    return ELF_LOAD_SUCCESS;
}

void* elf_lookup_symbol(const char *name) {
    // TODO add more shit here, such as searching shared libs etc
    return NULL;
}

Elf64_Shdr* elf_section_header(Elf64_Ehdr* header, uintptr_t base_address) {
    return (Elf64_Shdr*) (base_address + header->e_shoff);
}

Elf64_Shdr* elf_section(Elf64_Ehdr* header, uintptr_t base_address, Elf64_Xword idx) {
    return &elf_section_header(header, base_address)[idx];
}

char* elf_string_table(Elf64_Ehdr* header, uintptr_t base_address) {
    if (header->e_shstrndx == 0) {
        return NULL;
    }
    return (char*) (base_address + elf_section(header, base_address, header->e_shstrndx)->sh_offset);
}

char* elf_string(Elf64_Ehdr* header, uintptr_t base_address, int offset) {
    char* string_table = elf_string_table(header, base_address);
    if (string_table == NULL)
        return NULL;
    return string_table + offset;
}

uintptr_t elf_symbol_value(Elf64_Ehdr* header, uintptr_t base_address, Elf64_Xword table, Elf64_Xword idx,
        int32_t* error) {
    if (table == SHN_UNDEF || idx == SHN_UNDEF) {
        return 0;
    }

    Elf64_Shdr* symbol_table = elf_section(header, base_address, table);
    Elf64_Xword st_entries = symbol_table->sh_size / symbol_table->sh_entsize;
    if (idx >= st_entries) {
        *error = ELF_ERROR_RELOC_ERROR_SYM_INDEX_OUT_OF_RANGE;
        return 0;
    }

    Elf64_Xword symbol_address = base_address + symbol_table->sh_offset;
    Elf64_Sym* symbol = &((Elf64_Sym*)symbol_address)[idx];

    if (symbol->st_shndx == SHN_UNDEF) {
        // external symbol
        Elf64_Shdr* string_tab = elf_section(header, base_address, symbol_table->sh_link);
        const char* name = (const char*) (base_address + string_tab->sh_offset + symbol->st_name);
        void* target_symbol = elf_lookup_symbol(name);
        if (target_symbol == NULL) {
            // External symbol not found
            if (ELF64_ST_BIND(symbol->st_info) & STB_WEAK) {
                // weak symbol set to 0
                return 0;
            } else {
                *error = ELF_ERROR_RELOC_ERROR_SYM_EXTERNAL_NOT_FOUND;
                return 0;
            }
        } else {
            return (uintptr_t)target_symbol;
        }
    } else if (symbol->st_shndx == SHN_ABS) {
        // absolute symbol
        return symbol->st_value;
    } else {
        // internal symbol
        Elf64_Shdr* target = elf_section(header, base_address, symbol->st_shndx);
        return base_address + symbol->st_value + target->sh_offset;
    }
}

int32_t load_elf_stage_one(Elf64_Ehdr* header, uintptr_t base_address, proc_t* process) {
    Elf64_Shdr* section_header = elf_section_header(header, base_address);

    for (Elf64_Half i=0; i<header->e_shnum; i++) {
        Elf64_Shdr* section = &section_header[i];
        if (section->sh_type == SHT_NULL)
            continue;

        if (section->sh_flags & SHF_ALLOC) {
            if (section->sh_addr != 0) {
                if (section->sh_size == 0)
                    continue;

                // must appear at this address
                mmap_area_t** _mmap_area = request_va_hole(process, section->sh_addr, section->sh_size);
                mmap_area_t* mmap_area = *_mmap_area;
                if (mmap_area == NULL) {
                    return ELF_ERROR_SECTION_OVERLAPS;
                }
                mmap_area->mtype = program_data;
                if (!allocate(mmap_area->vastart, mmap_area->vaend-mmap_area->vastart, false, false, process->pml4)) {
                    return ELF_ERROR_ENOMEM;
                }
                if (section->sh_type == SHT_NOBITS) {
                    // nobits, just memset
                	memset_dpgs(process->pml4, (void*)mmap_area->vastart, 0, mmap_area->vaend-mmap_area->vastart);
                    section->sh_offset = base_address - mmap_area->vastart;
                } else if (section->sh_type == SHT_PROGBITS) {
                    // progbits, copy over
                    memcpy_dpgs(process->pml4, get_active_page(),
                    		(void*)mmap_area->vastart, (void*)base_address+section->sh_offset,
                            mmap_area->vaend-mmap_area->vastart);
                } else {
                    return ELF_ERROR_UNSUPPORTED_SECTION_TYPE_FIXADDR;
                }
            }
        }
    }

    for (Elf64_Half i=0; i<header->e_shnum; i++) {
        Elf64_Shdr* section = &section_header[i];

        if (section->sh_type == SHT_NULL)
            continue;

        if (section->sh_flags & SHF_ALLOC) {
            if (section->sh_addr == 0) {
                if (section->sh_size == 0)
                    continue;

                // must appear at this address
                mmap_area_t** _mmap_area = find_va_hole(process, section->sh_size, section->sh_addralign);
                mmap_area_t* mmap_area = *_mmap_area;
                if (mmap_area == NULL) {
                    return ELF_ERROR_SECTION_OVERLAPS;
                }
                mmap_area->mtype = program_data;
                if (!allocate(mmap_area->vastart, mmap_area->vaend-mmap_area->vastart, false, false, process->pml4)) {
                    return ELF_ERROR_ENOMEM;
                }
                if (section->sh_type == SHT_NOBITS) {
                    // nobits, just memset
                    memset_dpgs(process->pml4,
                    		(void*)mmap_area->vastart, 0, mmap_area->vaend-mmap_area->vastart);
                    section->sh_offset = base_address - mmap_area->vastart;
                } else if (section->sh_type == SHT_PROGBITS) {
                    // progbits, copy over
                	memcpy_dpgs(process->pml4, get_active_page(),
                			(void*)mmap_area->vastart, (void*)base_address+section->sh_offset,
                            mmap_area->vaend-mmap_area->vastart);
                    section->sh_addr = mmap_area->vastart;
                } else {
                    return ELF_ERROR_UNSUPPORTED_SECTION_TYPE_FIXADDR;
                }

            }
        }
    }

    return ELF_LOAD_SUCCESS;
}

int32_t load_elf_stage_two(Elf64_Ehdr* header, uintptr_t base_address, proc_t* process) {
    Elf64_Shdr* section_header = elf_section_header(header, base_address);

    // TODO: add relocation

    for (Elf64_Half i=0; i<header->e_shnum; i++) {
            Elf64_Shdr* section = &section_header[i];
            if (section->sh_type == SHT_NULL)
                continue;

            if (section->sh_flags & SHF_ALLOC) {
                if (section->sh_addr != 0) {
                    if ((section->sh_flags & SHF_WRITE) == 0)
                        mem_change_type(section->sh_addr, section->sh_size,
                                CHNG_TYPE_RW, false, process->pml4);
                }
            }
    }


    return ELF_LOAD_SUCCESS;
}

int32_t load_elf_general(Elf64_Ehdr* header, uintptr_t base_address, proc_t* process) {
    int32_t err;

    if ((err = load_elf_stage_one(header, base_address, process)) != ELF_LOAD_SUCCESS) {
        return err;
    }

    if ((err = load_elf_stage_two(header, base_address, process)) != ELF_LOAD_SUCCESS) {
        return err;
    }

    return ELF_LOAD_SUCCESS;
}

int32_t load_elf_exec(uintptr_t elf_file_data, proc_t* process) {
    Elf64_Ehdr* header = (Elf64_Ehdr*)elf_file_data;
    int32_t err;
    if ((err = check_elf_header(header)) != ELF_LOAD_SUCCESS) {
        return err;
    }

    if (header->e_type != ET_EXEC) {
        return ELF_ERROR_NOT_EXEC_EXEC_REQUIRED;
    }

    thread_t* thread = array_get_at(process->threads, 0);
    thread->last_rip = header->e_entry;

    if ((err = load_elf_general(header, elf_file_data, process)) != ELF_LOAD_SUCCESS) {
        return err;
    }

    // TODO: support alloc size
    size_t ssize = BASE_STACK_SIZE;
    mmap_area_t** _mmap_area = find_va_hole(process, ssize, 0x1000);
    mmap_area_t* mmap_area = *_mmap_area;
    if (mmap_area == NULL) {
        return ELF_ERROR_ENOMEM;
    }
    mmap_area->mtype = stack_data;
    if (!allocate(mmap_area->vastart, ssize, false, false, process->pml4)) {
        return ELF_ERROR_ENOMEM;
    }
    memset_dpgs(process->pml4, (void*)mmap_area->vastart, 0, ssize);
    thread->stack_bottom_address = mmap_area->vastart;
    thread->stack_top_address = mmap_area->vaend;
    thread->last_rsp = thread->stack_top_address;

    return ELF_LOAD_SUCCESS;
}
