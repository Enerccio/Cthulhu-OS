#include "heap.h"

int64_t tmp_heap;
uint64_t heap_start_address;
uint64_t heap_end_address;

uint64_t handle_kernel_memory(int required_amount){
	if (required_amount > 0){
		uint64_t old_heap_end = heap_end_address;
		heap_end_address += required_amount;
		allocate(old_heap_end, required_amount, true, false);
		return old_heap_end;
	} else {
		required_amount = 0-required_amount;
		uint64_t old_heap_end = heap_end_address;
		uint64_t new_heap_end = heap_end_address - required_amount;
		if (new_heap_end < heap_start_address){
			required_amount = heap_end_address - heap_start_address;
			new_heap_end = heap_start_address;
		}
		heap_end_address = new_heap_end;
		deallocate(heap_end_address, required_amount);
		return old_heap_end;
	}
}

aligned_ptr_t malign(size_t amount, uint16_t align){
	if (tmp_heap != 0){
		if (tmp_heap % align != 0){
			tmp_heap = tmp_heap + (align - (tmp_heap % align));
		}
		uint64_t head = tmp_heap;
		tmp_heap += amount;
		return (void*)head;
	}

	uint64_t unaligned = (uint64_t) malloc(amount+align+sizeof(uint64_t));
	uint64_t aligned = unaligned + sizeof(uint64_t);
	if (!(aligned % align == 0)){
		aligned = aligned + (align - (aligned % align));
	}
	uint64_t* ptr = (uint64_t*) aligned;
	*(ptr-1) = unaligned;
	return (void*)aligned;
}

void initialize_temporary_heap(uint64_t temp_heap_start){
	tmp_heap = temp_heap_start;
}

void initialize_standard_heap(){
	heap_end_address = heap_start_address = 0xfffffe8000000000;
	tmp_heap = 0;
}
