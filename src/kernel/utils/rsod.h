#pragma once

#include "../commons.h"

#define UNDEFINED_ERROR 						0
#define ERROR_NO_MEMORY_DETECTED				1

void init_errors();

void error(uint16_t ecode, uint64_t speccode, uint64_t speccode2, void* eaddress);
