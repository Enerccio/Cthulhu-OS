[EXTERN stack_top]

global Realm64
Realm64:
	mov rsp, stack_top
	mov rdi, rbx
	mov rsi, rsp
	xor rbp, rbp
	extern kernel_main
	call kernel_main

	cli
.hang64:
	hlt
	jmp .hang64
