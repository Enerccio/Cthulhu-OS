#pragma once

#include "../commons.h"

#define UNDEFINED_ERROR 						0
#define LOADER_ERROR_A20_ACTIVATION_FAILURE		1
#define LOADER_KERNEL_UNAVAILABLE				2

void init_errors();

void error(uint16_t ecode, uint64_t speccode, uint64_t speccode2, void* eaddress);
