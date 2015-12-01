[BITS 32]

GDT64:                           ; Global Descriptor Table (64-bit).
    .Null: equ $ - GDT64         ; The null descriptor.
    dw 0                         ; Limit (low).
    dw 0                         ; Base (low).
    db 0                         ; Base (middle)
    db 0                         ; Access.
    db 0                         ; Granularity.
    db 0                         ; Base (high).
    .Code: equ $ - GDT64         ; The code descriptor.
    dw 0                         ; Limit (low).
    dw 0                         ; Base (low).
    db 0                         ; Base (middle)
    db 10011010b                 ; Access (exec/read).
    db 00100000b                 ; Granularity.
    db 0                         ; Base (high).
    .Data: equ $ - GDT64         ; The data descriptor.
    dw 0                         ; Limit (low).
    dw 0                         ; Base (low).
    db 0                         ; Base (middle)
    db 10010010b                 ; Access (read/write).
    db 00000000b                 ; Granularity.
    db 0                         ; Base (high).
    .Pointer:                    ; The GDT-pointer.
    dw $ - GDT64 - 1             ; Limit.
    dq GDT64                     ; Base.

query_cpuid:
	pushfd               ; Store the FLAGS-register.
    pop eax              ; Restore the A-register.
    mov ecx, eax         ; Set the C-register to the A-register.
    xor eax, 1 << 21     ; Flip the ID-bit, which is bit 21.
    push eax             ; Store the A-register.
    popfd                ; Restore the FLAGS-register.
    pushfd               ; Store the FLAGS-register.
    pop eax              ; Restore the A-register.
    push ecx             ; Store the C-register.
    popfd                ; Restore the FLAGS-register.
    xor eax, ecx         ; Do a XOR-operation on the A-register and the C-register.
    jz nocpuid           ; The zero flag is set, no CPUID.
	mov eax, 0x80000000  ; Set the A-register to 0x80000000.
    cpuid                ; CPU identification.
    cmp eax, 0x80000001  ; Compare the A-register with 0x80000001.
    jb nolongmode
	mov eax, 0x80000001    ; Set the A-register to 0x80000001.
    cpuid                  ; CPU identification.
    test edx, 1 << 29      ; Test if the LM-bit, which is bit 29, is set in the D-register.
    jz nolongmode
    mov eax, 0
    ret
nocpuid:
	mov eax, -1
	ret
nolongmode:
	mov eax, 1
	ret

global piko_loader
piko_loader:
	push ebx
	push ebp
	call query_cpuid
	mov ecx, eax
	jecxz continue
	ret
continue:
	pop ebp
	pop ebx
	; we have long mode, continue operation
	mov edi, 0x1000    			 ; Set the destination index to 0x1000.
    mov cr3, edi       			 ; Set control register 3 to the destination index.
    xor eax, eax       			 ; Nullify the A-register.
    mov ecx, 4096      			 ; Set the C-register to 4096.
    rep stosd          			 ; Clear the memory.
    mov edi, cr3       			 ; Set the destination index to control register 3.
	mov DWORD [edi], 0x2003      ; Set the uint32_t at the destination index to 0x2003.
    add edi, 0x1000              ; Add 0x1000 to the destination index.
    mov DWORD [edi], 0x3003      ; Set the uint32_t at the destination index to 0x3003.
    add edi, 0x1000              ; Add 0x1000 to the destination index.
    mov DWORD [edi], 0x4003      ; Set the uint32_t at the destination index to 0x4003.
    add edi, 0x1000              ; Add 0x1000 to the destination index.

    mov edx, 0x00000003          ; Set the B-register to 0x00000003.
    mov ecx, 512                 ; Set the C-register to 512.

.SetEntry:
    mov DWORD [edi], edx         ; Set the uint32_t at the destination index to the D-register.
    add edx, 0x1000              ; Add 0x1000 to the D-register.
    add edi, 8                   ; Add eight to the destination index.
    loop .SetEntry               ; Set the next entry.

    mov eax, cr4                 ; Set the A-register to control register 4.
    or eax, 1 << 5               ; Set the PAE-bit, which is the 6th bit (bit 5).
    mov cr4, eax                 ; Set control register 4 to the A-register.

    mov ecx, 0xC0000080          ; Set the C-register to 0xC0000080, which is the EFER MSR.
    rdmsr                        ; Read from the model-specific register.
    or eax, 1 << 8               ; Set the LM-bit which is the 9th bit (bit 8).
    wrmsr                        ; Write to the model-specific register.

    mov eax, cr0                 ; Set the A-register to control register 0.
    or eax, 1 << 31 | 1 << 0     ; Set the PG-bit, which is the 31nd bit, and the PM-bit, which is the 0th bit.
    mov cr0, eax                 ; Set control register 0 to the A-register.

    lgdt [GDT64.Pointer]         ; Load the 64-bit global descriptor table.
    extern Realm64
    jmp GDT64.Code:Realm64       ; Set the code segment and enter 64-bit long mode.
