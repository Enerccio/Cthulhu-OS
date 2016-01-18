[GLOBAL dev_sys_0arg]
[GLOBAL dev_sys_0arg_e]
; 0 argument dev syscall
;
; extern uint64_t dev_sys_0arg(uint64_t syscallnum)
dev_sys_0arg:
    xor rsi, rsi
dev_sys_0arg_e:
    mov rax, rdi
    syscall
    test rsi, rsi
    je .ret
    mov qword [rsi], rdi
.ret:
    ret

[GLOBAL dev_sys_1arg]
[GLOBAL dev_sys_1arg_e]
; 1 argument dev syscall
;
; extern uint64_t dev_sys_1arg(uint64_t syscallnum, void* arg1)
dev_sys_1arg:
    xor rdx, rdx
dev_sys_1arg_e:
    mov rax, rdi
    mov rdi, rsi
    syscall
    test rdx, rdx
    je .ret
    mov qword [rdx], rdi
.ret:
    ret

[GLOBAL dev_sys_2arg]
[GLOBAL dev_sys_2arg_e]
; 2 argument dev syscall
;
; extern uint64_t dev_sys_2arg(uint64_t syscallnum, void* arg1, void* arg2)
dev_sys_2arg:
    xor rcx, rcx
dev_sys_2arg_e:
    mov rax, rdi
    mov rdi, rsi
    mov rsi, rdx
    syscall
    test rcx, rcx
    je .ret
    mov qword [rcx], rdi
.ret:
    ret

[GLOBAL dev_sys_3arg]
[GLOBAL dev_sys_3arg_e]
; 3 argument dev syscall
;
; extern uint64_t dev_sys_3arg(uint64_t syscallnum, void* arg1, void* arg2, void* arg3)
dev_sys_3arg:
    xor r8, r8
dev_sys_3arg_e:
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


[GLOBAL dev_sys_4arg]
[GLOBAL dev_sys_4arg_e]
; 4 argument dev syscall
;
; extern uint64_t dev_sys_4arg(uint64_t syscallnum, void* arg1, void* arg2, void* arg3, void* arg4)
dev_sys_4arg:
    xor r9, r9
dev_sys_4arg_e:
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

[GLOBAL dev_sys_5arg]
; 5 argument dev syscall
;
; extern uint64_t dev_sys_5arg(uint64_t syscallnum, void* arg1, void* arg2, void* arg3, void* arg4, void* arg5)
dev_sys_5arg:
    mov rax, rdi
    mov rdi, rsi
    mov rsi, rdx
    mov rdx, rcx
    mov rcx, r8
    mov r8, r9
    syscall
    ret
