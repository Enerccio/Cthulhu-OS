[BITS 32]

; Declare constants used for creating a multiboot header.
MBALIGN     equ  1<<0                   ; align loaded modules on page boundaries
MEMINFO     equ  1<<1                   ; provide memory map
FLAGS       equ  MBALIGN | MEMINFO      ; this is the Multiboot 'flag' field
MAGIC       equ  0x1BADB002             ; 'magic number' lets bootloader find the header
CHECKSUM    equ -(MAGIC + FLAGS)        ; checksum of above, to prove we are multiboot

section .multiboot
align 4
	dd MAGIC
	dd FLAGS
	dd CHECKSUM

section .bootstrap_stack, nobits
align 4
stack_bottom:
resb 409600
global stack_top
stack_top:

section .text
global _start
_start:
	cli
	mov esp, stack_top
	extern piko_loader
	call piko_loader

	cli
.hang:
	hlt
	jmp .hang
