[GLOBAL dev_sys_0arg]
; 0 argument dev syscall
;
; extern uint64_t dev_sys_0arg(uint64_t syscallnum)
dev_sys_0arg:
    mov rax, rdi
    int 0x81
    ret

[GLOBAL dev_sys_1arg]
; 1 argument dev syscall
;
; extern uint64_t dev_sys_1arg(uint64_t syscallnum, void* arg1)
dev_sys_1arg:
    mov rax, rdi
    mov rdi, rsi
    int 0x81
    ret

[GLOBAL dev_sys_2arg]
; 2 argument dev syscall
;
; extern uint64_t dev_sys_2arg(uint64_t syscallnum, void* arg1, void* arg2)
dev_sys_2arg:
    mov rax, rdi
    mov rdi, rsi
    mov rsi, rdx
    int 0x81
    ret

[GLOBAL dev_sys_3arg]
; 3 argument dev syscall
;
; extern uint64_t dev_sys_3arg(uint64_t syscallnum, void* arg1, void* arg2, void* arg3)
dev_sys_3arg:
    mov rax, rdi
    mov rdi, rsi
    mov rsi, rdx
    mov rdx, rcx
    int 0x81
    ret


[GLOBAL dev_sys_4arg]
; 4 argument dev syscall
;
; extern uint64_t dev_sys_4arg(uint64_t syscallnum, void* arg1, void* arg2, void* arg3, void* arg4)
dev_sys_4arg:
    mov rax, rdi
    mov rdi, rsi
    mov rsi, rdx
    mov rdx, rcx
    mov rcx, r8
    int 0x81
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
    int 0x81
    ret