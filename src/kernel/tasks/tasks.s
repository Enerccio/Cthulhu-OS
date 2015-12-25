[BITS 64]

[GLOBAL proc_spinlock_lock]
; Processor bound spinlock lock function
;
; extern proc_spinlock_lock(void* address)
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
; extern proc_spinlock_unlock(void* address)
proc_spinlock_unlock:
        mov qword [rdi], 0
        ret
