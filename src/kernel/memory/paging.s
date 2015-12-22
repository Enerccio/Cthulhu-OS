/*
 * The MIT License (MIT)
 * Copyright (c) 2015 Peter Vanusanik <admin@en-circle.com>
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 *
 * paging.s
 *  Created on: Dec 22, 2015
 *      Author: Peter Vanusanik
 *  Contents: paging memory for x86-64 helpful assemblies
 */

section .text
[BITS 64]

[GLOBAL detect_maxphyaddr]
detect_maxphyaddr:
	mov rax, 0x80000000
	cpuid
	mov rcx, 0x80000008
	cmp rcx, rax
	jle .find_maxphyaddr
.default_value:
	mov eax, 36									; minimal value for systems with long mode
	ret
.find_maxphyaddr:
	mov rax, 0x80000008
	cpuid										; get the information 80000008
	and eax, 0b00000000000000000000000001111111 ; get only 7 first bits
	ret

[GLOBAL get_active_page]
get_active_page:
	xor rax, rax
	mov rax, cr3
	ret

[GLOBAL set_active_page]
set_active_page:
	mov cr3, rdi
	ret

[GLOBAL invalidate_address]
invalidate_address:
	invlpg [rdx]
	ret
