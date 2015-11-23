#include "rsod.h"

const char* top_message = "We are sorry but kernel has suffered an error";
const char* top_message_2 = "from which it was unable to recover->";
const char* bottom_message = "Please check recent kernel updates or";
const char* bottom_message_2 = "hardware changes which can be the fault here.";
char* error_codes[8000];

void init_errors() {
	log_msg("Initializing error page");

	for (uint16_t i = 0; i < 8000; i++) {
		error_codes[i] = "";
	}

	error_codes[UNDEFINED_ERROR] = "UNDEFINED_ERROR";
	error_codes[ORDERED_ARRAY_ERROR] = "ORDERED_ARRAY_ERROR";
	error_codes[INTERRUPT_ERROR] = "INTERRUPT_ERROR";
	error_codes[UNHANDLED_INTERRUPT_ERROR] = "UNHANDLED_INTERRUPT_ERROR";
	error_codes[HEAP_ERROR_START_UNALIGNED] = "HEAP_ERROR_START_UNALIGNED";
	error_codes[HEAP_ERROR_REQ_SIZE_SMALLER_THAN_EX] =
			"HEAP_ERROR_REQ_SIZE_SMALLER_THAN_EX";
	error_codes[HEAP_ERROR_REQ_SIZE_TOO_BIG] = "HEAP_ERROR_REQ_SIZE_TOO_BIG";
	error_codes[HEAP_ERROR_REQ_SIZE_LARGER_THAN_EX] =
			"HEAP_ERROR_REQ_SIZE_LARGER_THAN_EX";
	error_codes[HEAP_ERROR_MEMORY_GARBAGE] = "HEAP_ERROR_MEMORY_GARBAGE";
	error_codes[PAGING_ERROR_NOMEM] = "PAGING_ERROR_NOMEM";
	error_codes[PAGE_FAULT] = "PAGE_FAULT";
	error_codes[ERROR_NO_VFS] = "ERROR_NO_VFS";
	error_codes[ERROR_UNABLE_TO_ACTIVATE_A20] = "ERROR_UNABLE_TO_ACTIVATE_A20";
}

void error(uint16_t ecode, uint32_t speccode, uint32_t speccode2, void* eaddress) {
	__asm  __volatile__ ("cli");

	kd_cclear(4);

	kd_setxy(10, 2);
	kd_cwrite(top_message, 4, 0);
	kd_setxy(10, 3);
	kd_cwrite(top_message_2, 4, 0);

	kd_setxy(16, 5);
	kd_cwrite("Error code: ", 4, 0);
	kd_cwrite_hex(ecode, 4, 0);

	kd_setxy(16, 6);
	kd_cwrite(error_codes[ecode], 4, 0);

	kd_setxy(16, 7);
	kd_cwrite("Specific code: ", 4, 0);
	kd_cwrite_hex(speccode, 4, 0);

	kd_setxy(16, 8);
	kd_cwrite("Additional code: ", 4, 0);
	kd_cwrite_hex(speccode2, 4, 0);

	kd_setxy(16, 10);
	kd_cwrite("Address: ", 4, 0);
	kd_cwrite_hex((uint32_t) eaddress, 4, 0);

	kd_setxy(10, 19);
	kd_cwrite(bottom_message, 4, 0);
	kd_setxy(10, 20);
	kd_cwrite(bottom_message_2, 4, 0);

	while (true)
		;
}

