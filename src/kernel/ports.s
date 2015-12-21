section .text
[BITS 64]

[GLOBAL outb]
[GLOBAL inb]
[GLOBAL inw]

outb:
	mov rcx, rdx
	mov rdx, rax
	mov rax, rcx
	out dx, al
	ret

inb:
	mov rdx, rax
	mov rax, 0
	in al, dx
	ret

inw:
	mov rdx, rax
	mov rax, 0
	in ax, dx
	ret
