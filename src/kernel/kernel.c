#include "commons.h"
#include "utils/rsod.h"
#include "memory/heap.h"
#include "memory/paging.h"

#include <stdlib.h>
#include <stdio.h>

void print_loader_revision() {
	kd_cwrite("PIKOKERNEL ", 0, 4);
	kd_cwrite(KERNEL_MAIN_VERSION, 0, 4);
	kd_cwrite(".", 0, 4);
	kd_cwrite(KERNEL_MINOR_VESION, 0, 4);
	kd_cwrite(", revision ", 0, 4);
	kd_cwrite(KERNEL_BUILD_VERSION, 0, 4);
	kd_cwrite(" - ", 0, 4);
	kd_cwrite(KERNEL_CODENAME, 0, 4);
	kd_newl();
}

void kernel_main(struct multiboot* mboot_addr, uint64_t heap_start)
{
	kd_clear();
	print_loader_revision();
	init_errors();

	initialize_temporary_heap(heap_start);
	initialize_paging(mboot_addr);

	allocate(0xCCCC0000, 0x10000, true, false);

	while (true) ;
}