[BITS 32]

[GLOBAL outb]
[GLOBAL inb]
[GLOBAL inw]

outb:
	mov ecx, edx
	mov edx, eax
	mov eax, ecx
	out dx, al
	ret

inb:
	mov edx, eax
	mov eax, 0
	in al, dx
	ret

inw:
	mov edx, eax
	mov eax, 0
	in ax, dx
	ret
