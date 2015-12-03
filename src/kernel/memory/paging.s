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
