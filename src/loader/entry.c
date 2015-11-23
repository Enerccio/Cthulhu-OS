#include "../shared/commons.h"
#include "preload.h"

void print_loader_revision() {
	kd_cwrite("PIKOKERNEL Loader ", 0, 4);
	kd_cwrite(LOADER_MAIN_VERSION, 0, 4);
	kd_cwrite(".", 0, 4);
	kd_cwrite(LOADER_MINOR_VESION, 0, 4);
	kd_cwrite(", revision ", 0, 4);
	kd_cwrite(LOADER_BUILD_VERSION, 0, 4);
	kd_cwrite(" - ", 0, 4);
	kd_cwrite(LOADER_CODENAME, 0, 4);
	kd_newl();
}

void piko_loader(struct multiboot* mboot) {
	kd_clear();
	print_loader_revision();
	init_errors();

	if (!perform_checks())
		return;

	while (true) ;
}
