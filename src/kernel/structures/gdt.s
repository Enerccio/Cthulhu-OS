[BITS 64]

[GLOBAL load_gdt]
; Loads gdt and new segments into descriptors
;
; extern void load_gdt(gdt_ptr_t* gdt, uint16_t tssid)
load_gdt:
    mov rax, rdi
    lgdt [rax]
    xor rax, rax
    mov ax, 16
    mov es, ax
    mov ds, ax
    mov ss, ax
    mov rax, rsi

    ltr ax

    add rsi, 16
    mov rax, rsi
    mov gs, ax

    push 8
    push .jmpt

    mov rax, rsp
    jmp far qword [rax]
.jmpt:
    add rsp, 8
    ret
