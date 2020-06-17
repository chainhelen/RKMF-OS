[BITS 32]


[SECTION .text]
global _start
global asmfunc1
global asmfunc2
global asmfunc3
global asmfunc4
extern 	cstart

_start:
	mov		ax, cs
	call 	cstart



asmfunc1:
mov byte [0xb8000+24*160+0],'C'
mov byte [0xb8000+24*160+1],0x0c
ret

asmfunc2:
mov byte [0xb8000+24*160+2],'-'
mov byte [0xb8000+24*160+3],0x0c
ret

asmfunc3:
mov byte [0xb8000+24*160+4],'>'
mov byte [0xb8000+24*160+5],0x0c
ret

asmfunc4:
mov byte [0xb8000+24*160+6],'A'
mov byte [0xb8000+24*160+7],0x0c
ret
