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
section .stack, nobits
align 16
stack_bottom64:
resb 0x5000
stack_top64:

section .text
global Realm64
Realm64:
	mov rsp, stack_top64
	mov rdi, rbx
	mov rsi, rsp
	xor rbp, rbp
	extern kernel_main
	call kernel_main
	cli
.hang64:
	hlt
	jmp .hang64
