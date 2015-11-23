#pragma once

#include "../commons.h"

#define UNDEFINED_ERROR 						0
#define ORDERED_ARRAY_ERROR 					1
#define INTERRUPT_ERROR 						2
#define UNHANDLED_INTERRUPT_ERROR 				3
#define HEAP_ERROR_START_UNALIGNED 				4
#define HEAP_ERROR_REQ_SIZE_SMALLER_THAN_EX 	5
#define HEAP_ERROR_REQ_SIZE_TOO_BIG				6
#define HEAP_ERROR_REQ_SIZE_LARGER_THAN_EX		7
#define HEAP_ERROR_MEMORY_GARBAGE				8
#define PAGING_ERROR_NOMEM						9
#define PAGE_FAULT								10
#define ERROR_NO_VFS							11
#define ERROR_UNABLE_TO_ACTIVATE_A20			12

void init_errors();

void error(uint16_t ecode, uint32_t speccode, uint32_t speccode2, void* eaddress);
