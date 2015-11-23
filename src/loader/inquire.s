[GLOBAL query_cpuid]

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


; Function: check_a20
;
; Purpose: to check the status of the a20 line in a completely self-contained state-preserving way.
;          The function can be modified as necessary by removing push's at the beginning and their
;          respective pop's at the end if complete self-containment is not required.
;
; Returns: 0 in ax if the a20 line is disabled (memory wraps around)
;          1 in ax if the a20 line is enabled (memory does not wrap around)

[GLOBAL query_a20]
query_a20:
	mov eax, 0

    pushf
    push ds
    push es
    push di
    push si

    mov al, byte [0x00000500]
    push ax

    mov al, byte [0xFFFF0510]
    push ax

    mov byte [0x00000500], 0x00
    mov byte [0xFFFF0510], 0xFF

    cmp byte [0x00000500], 0xFF

    pop ax
    mov byte [0xFFFF0510], al

    pop ax
    mov byte [0x00000500], al

    mov eax, 0
    je check_a20__exit

    mov eax, 1

check_a20__exit:
    pop si
    pop di
    pop es
    pop ds
    popf

    ret

[GLOBAL seta20_bios]
seta20_bios:
	mov ax, 0x2401
	int 0x15
	ret

[GLOBAL seta20_fast]
seta20_fast:
	in al, 0x92
	test al, 2
	jnz after
	or al, 2
	and al, 0xFE
	out 0x92, al
after:
	ret
