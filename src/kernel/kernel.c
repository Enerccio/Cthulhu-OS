#include "../shared/commons.h"

void _start()
{
	uint32_t initrd_loc = *((uint32_t*)0x2000000-4);

	log_msg("Kernel bootup complete");
	kd_write("Initrd file location: ");
	kd_write_hex(initrd_loc);
	kd_newl();
}
