#include "preload.h"

extern int32_t query_cpuid();
extern int32_t query_a20();
extern void seta20_bios();
extern void seta20_fast();

static void a20wait1(){
	while (inb(0x64) == 2) ;
}

static void a20wait2(){
	while (inb(0x64) == 1) ;
}

static void enable_a20(int step){
	if (step == 0){
		seta20_bios();
		return;
	} else {
		if (step == 1){
			uint8_t a;
			a20wait1();
			outb(0x64, 0xAD);
			a20wait1();
			outb(0x64, 0xD0);
			a20wait2();
			a = inb(0x60);
			a20wait1();
			outb(0x64, 0xD1);
			a20wait1();
			outb(0x60, a|2);
			a20wait1();
			outb(0x64, 0xAE);
			a20wait1();
		} else if (step == 100000){
			seta20_fast();
		} else if (step > 200000)
			error(LOADER_ERROR_A20_ACTIVATION_FAILURE, 0, 0, &enable_a20);
	}
}

bool perform_checks(){
	int32_t testcpu = query_cpuid();
	if (testcpu == -1){
		log_err("cpuid not supported");
		return false;
	} else if (testcpu == 1){
		log_err("long mode not supported");
		return false;
	} else {
		log_msg("Long mode supported, continuing loading kernel!");
	}

	log_msg("Checking whether a20 is enabled by bios.");

	if (!query_a20()){
		log_msg("A20 is not enabled, enabling");
		int step=0;
		do {
			enable_a20(step++);
		} while (!query_a20());
	}
	log_msg("A20 is enabled.");

	return true;
}
