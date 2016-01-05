[BITS 64]

[GLOBAL switch_to_usermode]
; Switches to usermode. Used when call was not from scheduler
;
; extern Noreturn void switch_to_usermode(uint64_t rdi, uint64_t rip, uint64_t rsp, uint64_t flags,
;                                         uint64_t rsi, uint64_t rdx);

switch_to_usermode:
    xor rbx, rbx
    xor rbp, rbp
    xor r12, r12
    xor r13, r13
    xor r14, r14
    xor r15, r15
    mov rax, 32
    mov ds, ax
    mov es, ax
    mov ds, ax
    ; todo add thread local
    mov fs, ax
    mov gs, ax
    push rax
    push rdx
    push rcx
    push 24
    push rsi
    mov rsi, r8 ; for main thread, this is char* argv
    mov rdx, r9 ; for main thread, this is char** envp;
    xor rax, rax
    xor rcx, rcx
    xor r10, r10
    xor r11, r11
    iret
