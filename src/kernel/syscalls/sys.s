[BITS 64]

%idefine sysretq o64 sysret

[EXTERN write_wmrsc]

[GLOBAL register_syscall_handler]
register_syscall_handler:
    ; set handler
    mov rdi, 0xC0000082
    mov rsi, syscall_enter
    call write_wmrsc

    ; set mask for flags
    mov rdi, 0xC0000084
    mov esi, ~(1<<10)
    call write_wmrsc

    ; write cs/ss
    mov rdi, 0xC0000081
    mov rsi, (24 << 16) + (8)
    sal rsi, 32
    call write_wmrsc

    ret

[EXTERN sys_handler]

[GLOBAL syscall_enter]
syscall_enter:
    mov rbx, rsp
    swapgs
    mov rsp, [gs:8]

    push 32|3
    push rbx
    xor rbx, rbx
    push r11
    xor r11, r11
    push 24|3
    push rcx
    xor rcx, rcx

    push 0 ; errcode and crap
    push rax

    push rax
    xor rax, rax
    xor rax, rax
    mov ax, fs
    push rax
    xor rax, rax
    mov ax, es
    push rax
    xor rax, rax
    mov ax, ds
    push rax ; 5
    mov rax, 0x10
    mov fs, ax
    mov ds, ax
    mov ss, ax
    mov es, ax

    push rdi
    push rsi
    push rdx
    push rbx
    push rcx ;10
    push rbp
    push r8
    push r9
    push r10
    push r11 ;15
    push r12
    push r13
    push r14
    push r15 ; 18
    mov rbx, rsp
    mov rdi, rsp                ; move "pointer" from rsp to rdi (first parameter)
    and rsp, 0xFFFFFFFFFFFFFFF0 ; align stack
    call sys_handler
    mov rsp, rbx
    pop r15
    pop r14
    pop r13
    pop r12
    pop r11 ;5
    pop r10
    pop r9
    pop r8
    pop rbp
    pop rcx ;10
    pop rbx
    pop rdx
    pop rsi
    pop rdi
    pop rax ;15
    mov ds, ax
    pop rax
    mov es, ax
    pop rax
    mov fs, ax
    pop rax ;18
    add rsp, 16

    pop rcx
    add rsp, 8
    pop r11
    pop rbx
    add rsp, 8

    swapgs
    mov rsp, rbx
    sysretq
