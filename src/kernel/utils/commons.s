[BITS 64]

[GLOBAL kp_halt]
kp_halt:
.loop:
	jmp .loop
