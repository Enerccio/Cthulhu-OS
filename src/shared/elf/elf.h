#pragma once

#include "elfdef.h"
#include "../utils/rsod.h"

#define ELF_HEADER_INCORRECT (void*)0
#define ELF_HEADER_NOENTRIES (void*)1
#define ELF_KP_EMPTY         (void*)2

void elf_load_kernel(Elf64_Ehdr* header);
