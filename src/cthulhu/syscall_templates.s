[GLOBAL sys_0arg]
[GLOBAL sys_0arg_e]
; 0 argument syscall
;
; extern uint64_t sys_0arg(uint64_t num)
sys_0arg:
    xor rsi, rsi
sys_0arg_e:
    mov rax, rdi
    push rbx
    syscall
    pop rbx
    test rsi, rsi
    je .ret
    mov qword [rsi], rdi
.ret:
    ret

[GLOBAL sys_1arg]
[GLOBAL sys_1arg_e]
; 1 argument syscall
sys_1arg:
    xor rdx, rdx
sys_1arg_e:
    mov rax, rdi
    mov rdi, rsi
    push rbx
    syscall
    pop rbx
    test rdx, rdx
    je .ret
    mov qword [rdx], rdi
.ret:
    ret

[GLOBAL sys_2arg]
[GLOBAL sys_2arg_e]
; 2 argument syscall
sys_2arg:
    xor r8, r8
sys_2arg_e:
    mov rax, rdi
    mov rdi, rsi
    mov rsi, rdx
    push rbx
    syscall
    pop rbx
    test r8, r8
    je .ret
    mov qword [r8], rdi
.ret:
    ret

[GLOBAL sys_3arg]
[GLOBAL sys_3arg_e]
; 3 argument syscall
sys_3arg:
    xor r8, r8
sys_3arg_e:
    mov rax, rdi
    mov rdi, rsi
    mov rsi, rdx
    mov rdx, rcx
    push rbx
    syscall
    pop rbx
    test r8, r8
    je .ret
    mov qword [r8], rdi
.ret:
    ret


[GLOBAL sys_4arg]
[GLOBAL sys_4arg_e]
; 4 argument syscall
sys_4arg:
    xor r9, r9
sys_4arg_e:
    mov rax, rdi
    mov rdi, rsi
    mov rsi, rdx
    mov rdx, rcx
    mov r8, r8
    push rbx
    syscall
    pop rbx
    test r9, r9
    je .ret
    mov qword [r9], rdi
.ret:
    ret

[GLOBAL sys_5arg]
; 5 argument syscall
sys_5arg:
    mov rax, rdi
    mov rdi, rsi
    mov rsi, rdx
    mov rdx, rcx
    mov r8, r8
    mov r9, r9
    push rbx
    syscall
    pop rbx
    ret
