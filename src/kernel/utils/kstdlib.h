#pragma once

#include "../commons.h"

void* memcpy(void* destination, const void* source, size_t num);

void* memset(void* pointer, uint8_t value, size_t num);

uint32_t strlen(const char* string);

cmpresult_t strcmp(const char* first, const char* second);

char*strcpy(char*target, const char*source);

uint32_t rand_number(int32_t limit);

const char* hextochar(uint32_t num) UNSAFE_THREAD_CALL SINGLETON_RETURN;

uint32_t int_hash_function(void* integer);

bool int_cmpr_function(void* a, void* b);
