 ;
 ; The MIT License (MIT)
 ; Copyright (c) 2015 Peter Vanusanik <admin@en-circle.com>
 ;
 ; Permission is hereby granted, free of charge, to any person obtaining a copy
 ; of this software and associated documentation files (the "Software"), to deal in
 ; the Software without restriction, including without limitation the rights to use, copy,
 ; modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and
 ; to permit persons to whom the Software is furnished to do so, subject to the following conditions:
 ;
 ; The above copyright notice and this permission notice shall be included in all copies
 ; or substantial portions of the Software.
 ;
 ; THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED,
 ; INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A
 ; PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
 ; HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF
 ; CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE
 ; OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 ;
 ; paging.s
 ;  Created on: Dec 22, 2015
 ;      Author: Peter Vanusanik
 ;  Contents: paging helper assembly functions for x86-64 paging
 ;

section .text
[BITS 64]

[GLOBAL detect_maxphyaddr]
; Detects maxphyaddr attribute of paging strucutres
; or returns 36 if it can't be determined
;
; extern uint64_t detect_maxphyaddr()
detect_maxphyaddr:
    mov rax, 0x80000000
    cpuid
    mov rcx, 0x80000008
    cmp rcx, rax
    jle .find_maxphyaddr
.default_value:
    mov eax, 36                                 ; minimal value for systems with long mode
    ret
.find_maxphyaddr:
    mov rax, 0x80000008
    cpuid                                       ; get the information 80000008
    and eax, 0b00000000000000000000000001111111 ; get only 7 first bits
    ret

[GLOBAL is_1GB_paging_supported]
; Returns non-zero if 1GB paging is supported on this platform
;
; uint64_t is_1GB_paging_supported()
is_1GB_paging_supported:
    mov rax, 0x80000001
    cpuid
    mov eax, edx
    and eax, 1<<26
    ret

[GLOBAL get_active_page]
; Returns active page from cr3
;
; void* get_active_page()
get_active_page:
    xor rax, rax
    mov rax, cr3
    ret

[GLOBAL set_active_page]
; Sets active page to cr3
;
; extern void set_active_page(void* page)
set_active_page:
    mov cr3, rdi
    ret

[GLOBAL invalidate_address]
; Invalidates address in TLB
;
; extern void invalidate_address(void* address)
invalidate_address:
    invlpg [rdi]
    ret

[GLOBAL get_faulting_address]
; Returns faulting address from cr2
;
; extern void* get_faulting_address()
get_faulting_address:
    xor rax, rax
    mov rax, cr2
    ret
