/*
 * The MIT License (MIT)
 * Copyright (c) 2015 Peter Vanusanik <admin@en-circle.com>
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 *
 * kernel.c
 *  Created on: Dec 22, 2015
 *      Author: Peter Vanusanik
 *  Contents: main kernel entry function (in long mode in c).
 */
#include "ports/ports.h"
#include "commons.h"
#include "utils/rsod.h"
#include "memory/heap.h"
#include "memory/paging.h"

#include <stdlib.h>
#include <stdio.h>
#include <sys/kernel.h>

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

void kernel_main(struct multiboot* mboot_addr, uint64_t heap_start){
	volatile bool bbreak = true;
	while (bbreak)
		;

	initialize_ports();

	kd_clear();
	print_loader_revision();
	init_errors();

	initialize_temporary_heap(heap_start);
	initialize_paging(mboot_addr);
	initialize_standard_heap();

	log_msg("Paging memory and kernel heap initialized");
	__initialize_kclib();
	log_msg("KCLib initialized");

	while (true) ;
}
