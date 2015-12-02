#pragma once

#include "../commons.h"

char*strcpy(char*target, const char*source);

uint32_t rand_number(int32_t limit);

const char* hextochar(uint32_t num) UNSAFE_THREAD_CALL SINGLETON_RETURN;

uint32_t int_hash_function(void* integer);

bool int_cmpr_function(void* a, void* b);
