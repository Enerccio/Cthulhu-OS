#include "paging.h"

uint64_t maxphyaddr;

extern uint64_t detect_maxphyaddr();

void initialize_paging()
{
	printf("Initializing paging memory... \n");
	maxphyaddr = detect_maxphyaddr();
	printf("... Max physical address bits: %ld \n", maxphyaddr);

}
