#pragma once

#include "../commons.h"
#include "../utils/rsod.h"
#include "paging.h"

#include <stdlib.h>

void initialize_temporary_heap(uint64_t temp_heap_start);
void initialize_standard_heap();
void* malign(size_t amount, uint16_t align);
