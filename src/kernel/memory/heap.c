#include "heap.h"

uint64_t tmp_heap;

uint64_t handle_kernel_memory(int required_amount)
{
	if (tmp_heap != 0)
	{
		// temporary allocation
		if (required_amount < 0)
			return tmp_heap;
		else {
			uint64_t head = tmp_heap;
			tmp_heap += required_amount;
			return head;
		}
	}

	return -1;
}

void initialize_temporary_heap(uint64_t temp_heap_start)
{
	tmp_heap = temp_heap_start;
}

aligned_ptr_t malign(size_t amount, uint16_t align)
{
	uint64_t unaligned = (uint64_t) malloc(amount+align+sizeof(uint64_t));
	uint64_t aligned = unaligned + sizeof(uint64_t);
	if (!(aligned % align == 0)){
		aligned = aligned + (align - (aligned % align));
	}
	uint64_t* ptr = (uint64_t*) aligned;
	*(ptr-1) = unaligned;
	return (void*)aligned;
}

void freealign(aligned_ptr_t aligned_pointer){
	uint64_t* ptr = (uint64_t*) aligned_pointer;
	uint64_t unaligned_address = *(ptr-1);
	free((void*)unaligned_address);
}
