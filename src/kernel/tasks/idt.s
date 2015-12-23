[BITS 64]

%macro ISR_NOERRCODE 1  ; define a macro, taking one parameter
  [GLOBAL isr%1]        ; %1 accesses the first parameter.
  isr%1:
  	push rax ; garbage push, to simulate errcode
    push rdi
  	push rsi
  	push rdx
  	mov rdi, 0
  	mov rsi, %1
    jmp isr_common_stub
%endmacro

%macro ISR_ERRCODE 1
  [GLOBAL isr%1]
  isr%1:
  	push rdi
  	mov rdi, [rsp+8] ; move stack value of error to rdi
  	push rsi
  	push rdx
  	mov rsi, %1
    jmp isr_common_stub
%endmacro

%macro IRQ 2
  [GLOBAL irq%1]
  irq%1:
    push rax ; garbage push, to simulate errcode
  	push rdi
  	push rsi
  	push rdx
  	mov rdi, 0
  	mov rsi, %1
    jmp irq_common_stub
%endmacro

ISR_NOERRCODE 0
ISR_NOERRCODE 1
ISR_NOERRCODE 2
ISR_NOERRCODE 3
ISR_NOERRCODE 4
ISR_NOERRCODE 5
ISR_NOERRCODE 6
ISR_NOERRCODE 7
ISR_ERRCODE 8
ISR_NOERRCODE 9
ISR_ERRCODE 10
ISR_ERRCODE 11
ISR_ERRCODE 12
ISR_ERRCODE 13
ISR_ERRCODE 14
ISR_NOERRCODE 15
ISR_NOERRCODE 16
ISR_NOERRCODE 17
ISR_NOERRCODE 18
ISR_NOERRCODE 19
ISR_NOERRCODE 20
ISR_NOERRCODE 21
ISR_NOERRCODE 22
ISR_NOERRCODE 23
ISR_NOERRCODE 24
ISR_NOERRCODE 25
ISR_NOERRCODE 26
ISR_NOERRCODE 27
ISR_NOERRCODE 28
ISR_NOERRCODE 29
ISR_NOERRCODE 30
ISR_NOERRCODE 31
ISR_NOERRCODE 128

IRQ   0,    32
IRQ   1,    33
IRQ   2,    34
IRQ   3,    35
IRQ   4,    36
IRQ   5,    37
IRQ   6,    38
IRQ   7,    39
IRQ   8,    40
IRQ   9,    41
IRQ   10,   42
IRQ   11,   43
IRQ   12,   44
IRQ   13,   45
IRQ   14,   46
IRQ   15,   47

%macro PUSH_GEN_REGS 0
    push rax
	push rbx
	push rcx
	push rbp
	push r8
	push r9
	push r10
	push r11
	push r12
	push r13
	push r14
	push r15
%endmacro

%macro POP_GEN_REGS 0
    pop r15
    pop r14
    pop r13
    pop r12
    pop r11
    pop r10
    pop r9
    pop r8
    pop rbp
    pop rcx
    pop rbx
    pop rax
%endmacro

[EXTERN isr_handler]
[EXTERN irq_handler]

isr_common_stub:
	PUSH_GEN_REGS
    mov rdx, rsp ; move "pointer" from rsp to rdx (third parameter)
	mov rax, isr_handler
    call rax
    POP_GEN_REGS
	jmp end_common_stub

irq_common_stub:
	PUSH_GEN_REGS
	mov rdx, rsp ; move "pointer" from rsp to rdx (third parameter)
	mov rax, irq_handler
    call rax
	POP_GEN_REGS
    jmp end_common_stub

end_common_stub:
	pop rdi
  	pop rsi
  	pop rdx
  	add rsp, 8 ; remove garbage/error
	iretq

[GLOBAL idt_flush]
idt_flush:
   mov rax, rdi      ; Get the pointer to the IDT, passed as a parameter.
   lidt [rax]        ; Load the IDT pointer.
   ret
