#include <sys/external.h>

#include "../utils/rsod.h"
#include "../memory/paging.h"
#include "../memory/heap.h"

__noreturn __kclib_assert_failure_k(uint32_t lineno, const char* file, const char* func){
	error(ERROR_INTERNAL_LIBC, 0, lineno, 0);
}

void* __kclib_heap_start(){
	return (void*)heap_start_address;
}

void*	  __kclib_allocate(uintptr_t afrom, size_t aamount){
	allocate(afrom, aamount, true, false);
	return (void*)afrom;
}

void 	  __kclib_deallocate(uintptr_t afrom, size_t aamount){
	deallocate(afrom, aamount);
}

uint8_t    __kclib_isreclaimed(uintptr_t afrom, size_t aamount){
	bool reclaimed = true;
	for (uint64_t addr = afrom; addr < afrom + aamount; addr += 0x1000) {
		bool creclaimed = allocated(addr);
		if (creclaimed == false){
			reclaimed = false;
		}
	}
	return reclaimed;
}

void*	  __kclib_open_std_stream(uint8_t request_mode){
	return (void*)request_mode;
}

ptrdiff_t  __kclib_send_data(void* stream, uint8_t* array, size_t buffer_size){
	return 0;
}

ptrdiff_t  __kclib_read_data(void* stream, uint8_t* buffer, size_t read_amount){
	return 0;
}

ptrdiff_t  __kclib_unread_data(void* stream, uint8_t* buffer, size_t unread_amount){
	return 0;
}
