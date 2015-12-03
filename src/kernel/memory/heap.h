#pragma once

#include "../commons.h"
#include "../utils/rsod.h"
#include "paging.h"

#include <stdlib.h>

void initialize_temporary_heap(uint64_t temp_heap_start);

void* malign(size_t amount, uint16_t align);
void freealign(aligned_ptr_t aligned_pointer);
void* malign_p(size_t amount, uint16_t align);
void freealign_p(void* ptr);
