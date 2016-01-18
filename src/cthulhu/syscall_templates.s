[GLOBAL sys_0arg]
[GLOBAL sys_0arg_e]
; 0 argument dev syscall
;
; extern uint64_t sys_0arg(uint64_t syscallnum)
sys_0arg:
    xor rsi, rsi
sys_0arg_e:
    mov rax, rdi
    syscall
    test rsi, rsi
    je .ret
    mov qword [rsi], rdi
.ret:
    ret

[GLOBAL sys_1arg]
[GLOBAL sys_1arg_e]
; 1 argument dev syscall
;
; extern uint64_t sys_1arg(uint64_t syscallnum, void* arg1)
sys_1arg:
    xor rdx, rdx
sys_1arg_e:
    mov rax, rdi
    mov rdi, rsi
    syscall
    test rdx, rdx
    je .ret
    mov qword [rdx], rdi
.ret:
    ret

[GLOBAL sys_2arg]
[GLOBAL sys_2arg_e]
; 2 argument dev syscall
;
; extern uint64_t sys_2arg(uint64_t syscallnum, void* arg1, void* arg2)
sys_2arg:
    xor rcx, rcx
sys_2arg_e:
    mov rax, rdi
    mov rdi, rsi
    mov rsi, rdx
    syscall
    test rcx, rcx
    je .ret
    mov qword [rcx], rdi
.ret:
    ret

[GLOBAL sys_3arg]
[GLOBAL sys_3arg_e]
; 3 argument dev syscall
;
; extern uint64_t sys_3arg(uint64_t syscallnum, void* arg1, void* arg2, void* arg3)
sys_3arg:
    xor r8, r8
sys_3arg_e:
    mov rax, rdi
    mov rdi, rsi
    mov rsi, rdx
    mov rdx, rcx
    syscall
    test r8, r8
    je .ret
    mov qword [r8], rdi
.ret:
    ret


[GLOBAL sys_4arg]
[GLOBAL sys_4arg_e]
; 4 argument dev syscall
;
; extern uint64_t sys_4arg(uint64_t syscallnum, void* arg1, void* arg2, void* arg3, void* arg4)
sys_4arg:
    xor r9, r9
sys_4arg_e:
    mov rax, rdi
    mov rdi, rsi
    mov rsi, rdx
    mov rdx, rcx
    mov rcx, r8
    syscall
    test r9, r9
    je .ret
    mov qword [r9], rdi
.ret:
    ret

[GLOBAL sys_5arg]
; 5 argument dev syscall
;
; extern uint64_t sys_5arg(uint64_t syscallnum, void* arg1, void* arg2, void* arg3, void* arg4, void* arg5)
sys_5arg:
    mov rax, rdi
    mov rdi, rsi
    mov rsi, rdx
    mov rdx, rcx
    mov rcx, r8
    mov r8, r9
    syscall
    ret
