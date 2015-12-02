#pragma once

#include "../commons.h"

#define UNDEFINED_ERROR 						0

void init_errors();

void error(uint16_t ecode, uint64_t speccode, uint64_t speccode2, void* eaddress);
