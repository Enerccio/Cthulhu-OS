
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
