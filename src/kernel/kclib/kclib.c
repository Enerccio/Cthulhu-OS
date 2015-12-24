/*
 * The MIT License (MIT)
 * Copyright (c) 2015 Peter Vanusanik <admin@en-circle.com>
 *
 * Permission is hereby granted, free of charge, to any person obtaining
 * a copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included
 * in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED,
 * INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR
 * PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE
 * LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
 * TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE
 * USE OR OTHER DEALINGS IN THE SOFTWARE.
 *
 * kclib.c
 *  Created on: Dec 22, 2015
 *      Author: Peter Vanusanik
 *  Contents: implementing external definitions for kclib
 */
#include <sys/external.h>

#include "../utils/rsod.h"
#include "../memory/paging.h"
#include "../memory/heap.h"
#include "../utils/textinput.h"

__noreturn __kclib_assert_failure_k(uint32_t lineno, const char* file, const char* func) {
    error(ERROR_INTERNAL_LIBC, 0, lineno, 0);
}

void* __kclib_heap_start() {
    return (void*)heap_start_address;
}

void*     __kclib_allocate(size_t aamount) {
    uint64_t ha = heap_start_address;
    allocate(heap_start_address, aamount, true, false);
    heap_start_address += aamount; // fix deallocation
    return (void*)ha;
}

void      __kclib_deallocate(uintptr_t afrom, size_t aamount) {
    deallocate(afrom, aamount);
}

uint8_t    __kclib_isreclaimed(uintptr_t afrom, size_t aamount) {
    bool reclaimed = true;
    for (uint64_t addr = afrom; addr < afrom + aamount; addr += 0x1000) {
        bool creclaimed = allocated(addr);
        if (creclaimed == false){
            reclaimed = false;
        }
    }
    return reclaimed;
}

void*     __kclib_open_std_stream(uint8_t request_mode) {
    return (void*)(uintptr_t)request_mode;
}

ptrdiff_t  __kclib_send_data(void* stream, uint8_t* array, size_t buffer_size) {
    if (stream == ((void*)1)){
        for (size_t i=0; i<buffer_size; i++){
            kd_put(array[i]);
        }
        return buffer_size;
    }
    return 0;
}

ptrdiff_t  __kclib_read_data(void* stream, uint8_t* buffer, size_t read_amount) {
    return 0;
}
