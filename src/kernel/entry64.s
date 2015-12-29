 ;
 ; The MIT License (MIT)
 ; Copyright (c) 2015 Peter Vanusanik <admin@en-circle.com>
 ;
 ; Permission is hereby granted, free of charge, to any person
 ; obtaining a copy of this software and associated documentation
 ; files (the "Software"), to deal in the Software without restriction,
 ; including without limitation the rights to use, copy, modify, merge, publish,
 ; distribute, sublicense, and/or sell copies of the Software, and to permit persons
 ; to whom the Software is furnished to do so, subject to the following conditions:
 ;
 ; The above copyright notice and this permission notice shall be included in all
 ; copies or substantial portions of the Software.
 ;
 ; THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED,
 ; INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR
 ; PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE
 ; FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,
 ; ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 ;
 ; entry64.s
 ;  Created on: Dec 22, 2015
 ;      Author: Peter Vanusanik
 ;  Contents: long mode entry
 ;
[BITS 64]
section .stack, nobits
align 16
stack_bottom64:
resb 0x5000
stack_top64:

section .text
[GLOBAL Realm64]
Realm64:
    mov rsp, stack_top64
    mov rdi, rbx
    mov rsi, heap
    xor rbp, rbp
    extern kernel_main
    call kernel_main
    cli
.hang64:
    hlt
    jmp .hang64

[GLOBAL APRealm64]
APRealm64:
    mov rbx, (0xFEE00000 + 2 * 0x10) + (0xFFFF000000000000 + (509<<39))
    mov rax, [rbx]
    shr rax, 24
    and rax, 0xFF
    mov rdi, rax ; rdi now holds processor id
    extern cpuid_to_cputord
    mov rbx, cpuid_to_cputord
    mov rdx, 4
    mul rdx
    xor rcx, rcx
    mov ecx, dword [rbx + rax] ; rcx contains proc_id->table_id
    extern cpus
    mov rbx, [cpus] ; rbx contains pointer to array->data
    mov rax, 8
    mul rcx
    add rax, [rbx] ; rax contains pointer to cpu
    mov rsi, [rax] ; rsi contains pointer to stack
    mov rsp, [rsi]
    xor rbp, rbp
    extern ap_main
    call ap_main
    cli
.hangap:
    hlt
    jmp .hangap

section .temp_heap_storage, nobits
align 16
heap:
resb 0x40000

section .map_frame, nobits
map_frame:
resb 0x2000
