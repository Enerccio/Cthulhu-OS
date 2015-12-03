[GLOBAL detect_maxphyaddr]

detect_maxphyaddr:
	mov rax, 0x80000000
	cpuid
	mov rcx, 0x80000008
	cmp rcx, rax
	jle find_maxphyaddr
default_value:
	mov eax, 36
	ret
find_maxphyaddr:
	mov rax, 0x80000008
	cpuid
	and eax, 0b00000000000000000000000001111111
	ret
