#include "../shared/commons.h"
#include "../shared/elf/elf.h"
#include "preload.h"

void print_loader_revision(int x) {
	kd_cwrite("PIKOKERNEL Loader ", x, 4);
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
	print_loader_revision(0);
	init_errors();

	if (!perform_checks())
		return;

	if (mboot->mods_count == 0){
		log_err("No kernel specified!");
		return;
	}

	if (mboot->mods_count == 1){
		log_err("No initramfs specified!");
		return;
	}

	if (mboot->mods_count > 2){
		log_warn("more than 2 modules specified, only first two modules will be used");
	}

	uint32_t kernel_loc = *((uint32_t*)mboot->mods_addr);
	uint32_t initramfs_loc = *((uint32_t*)mboot->mods_addr+16);

	// store initramfs to location 0x2000000-4
	uint32_t* initramfs_ptr = (uint32_t*)0x2000000-4;
	*initramfs_ptr = initramfs_loc;

	elf_load_kernel((Elf64_Ehdr*) kernel_loc);
}
