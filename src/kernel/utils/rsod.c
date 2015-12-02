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

}

void error(uint16_t ecode, uint64_t speccode, uint64_t speccode2, void* eaddress) {
	__asm  __volatile__ ("cli");

	kd_cclear(4);

	kd_setxy(10, 2);
	kd_cwrite(top_message, 4, 0);
	kd_setxy(10, 3);
	kd_cwrite(top_message_2, 4, 0);

	kd_setxy(16, 5);
	kd_cwrite("Error code: ", 4, 0);
	kd_cwrite_hex64(ecode, 4, 0);

	kd_setxy(16, 6);
	kd_cwrite(error_codes[ecode], 4, 0);

	kd_setxy(16, 7);
	kd_cwrite("Specific code: ", 4, 0);
	kd_cwrite_hex64(speccode, 4, 0);

	kd_setxy(16, 8);
	kd_cwrite("Additional code: ", 4, 0);
	kd_cwrite_hex64(speccode2, 4, 0);

	kd_setxy(16, 10);
	kd_cwrite("Address: ", 4, 0);
	kd_cwrite_hex64((uint64_t) eaddress, 4, 0);

	kd_setxy(10, 19);
	kd_cwrite(bottom_message, 4, 0);
	kd_setxy(10, 20);
	kd_cwrite(bottom_message_2, 4, 0);

	while (true)
		;
}

