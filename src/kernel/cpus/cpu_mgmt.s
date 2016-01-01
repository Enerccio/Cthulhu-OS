[BITS 64]

[GLOBAL wait_until_activated]
; waits until rax changes to wait code
;
; extern void wait_until_activated(uint64_t wait_code)
wait_until_activated:
    mov rax, 0
.test:
    hlt
    cmp rax, rdi
    jne .test
    ret


[GLOBAL proc_spinlock_lock]
; Processor bound spinlock lock function
;
; extern void proc_spinlock_lock(void* address)
proc_spinlock_lock:
        mov     rax, 1
lock    xchg    [rdi], rax
        cmp     rax, 0
        je      .spinlock_escape
        pause
        jmp     proc_spinlock_lock
.spinlock_escape:
        ret

[GLOBAL proc_spinlock_unlock]
; Processor bound spinlock unlock function
;
; extern void proc_spinlock_unlock(void* address)
proc_spinlock_unlock:
        mov qword [rdi], 0
        ret
