section .init
[GLOBAL _init]
_init:
   push rbp
   mov rbp, rsp

section .fini
[GLOBAL _fini]
_fini:
   push rbp
   mov rbp, rsp
