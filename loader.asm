%include    "const.inc"
jmp		newprogram

gdt_size dw 32-1   ; GDT 表大小
gdt_base dd 0x00007e00 ;GDT的物理地址

msgstep2:			db	'step2: now jmp out mbr','$'
mesmem2:			db	'will visit memory address is---','$'
msgcs2:				db	'cs:????H','$'

msgstep3:			db	'Step3:now enter protect mode', '$'

newprogram:
mov		ax,newseg
mov		ds,ax

call	outmbr
call	showcsnew

call	showprotect

mov		ax,dptseg
mov		es,ax
call	createdpt

jmp 	next

createdpt:
lgdt	[gdt_size]

;创建#0描述符，空描述
mov		dword	[es:0x00],0x00
mov		dword	[es:0x04],0x00

;创建#1描述符,	保护模式下的段描述符
mov		dword	[es:0x08],0x8000ffff
mov		dword	[es:0x0c],0x00409800

;缓冲区
mov		dword	[es:0x10],0x0000ffff
mov		dword	[es:0x14],0x00c09200

;#3
mov		dword	[es:0x18],0x00007a00
mov		dword	[es:0x1c],0x00409600
ret

outmbr:
call	newlinenew
mov		si,msgstep2
call	printstrnew
call	newlinenew
mov		si,mesmem2
call	printstrnew
ret

showprotect:
call	newlinenew
mov		si,msgstep3
call	newlinenew
call	printstrnew
call	newlinenew
ret

showcsnew:
	mov		ax,cs
	mov		dl,ah
	call	hl4bitnew

	mov		dl,bh
	call	asciinew
	mov		[msgcs2+3],dl

	mov		dl,bl
	call	asciinew
	mov		[msgcs2+4],dl

	mov		dl,al
	call	hl4bitnew

	mov		dl,bh
	call	asciinew
	mov		[msgcs2+5],dl

	mov		dl,bl
	call	asciinew
	mov		[msgcs2+6],dl

	mov		si,msgcs2
	call	printstrnew

	ret

printstrnew:
	mov		al,[si]
	cmp		al,'$'
	je		disovernew
	mov		ah,0eh
	int		10h
	inc		si
	jmp		printstrnew
disovernew:
	ret

newlinenew:
	mov		ah,0eh
	mov		al,0dh
	int		10h
	mov		al,0ah
	int		10h
	ret

asciinew:
	cmp		dl,9
	jg		letternew
	add  	dl,30h
	ret
letternew:
	add		dl,37h
	ret

hl4bitnew:
	mov		dh,dl
	mov		bl,dl
	shr		dh,1
	shr		dh,1
	shr		dh,1
	shr		dh,1
	mov		bh,dh
	and		bl,0fh
	ret

next:
in		al,0x92
or		al,0000_0010B
out		0x92,al

cli

mov		eax,cr0
or		eax,1
mov		cr0,eax

jmp dword	0x0008:inprotectmode

[bits 32]
inprotectmode:
mov		ax,00000000000_10_000B
mov		ds,ax

mov		byte 	[0xb8000+22*160+0x00],'P'
mov		byte 	[0xb8000+22*160+0x01],0x0c
mov		byte 	[0xb8000+22*160+0x02],'R'
mov		byte 	[0xb8000+22*160+0x03],0x0c
mov		byte 	[0xb8000+22*160+0x04],'O'
mov		byte 	[0xb8000+22*160+0x05],0x0c
mov		byte 	[0xb8000+22*160+0x06],'T'
mov		byte 	[0xb8000+22*160+0x07],0x0c
mov		byte 	[0xb8000+22*160+0x08],'E'
mov		byte 	[0xb8000+22*160+0x09],0x0c
mov		byte 	[0xb8000+22*160+0x0a],'C'
mov		byte 	[0xb8000+22*160+0x0b],0x0c
mov		byte 	[0xb8000+22*160+0x0c],'T'
mov		byte 	[0xb8000+22*160+0x0d],0x0c
mov		byte 	[0xb8000+22*160+0x0e],'-'
mov		byte 	[0xb8000+22*160+0x0f],0x0c
mov		byte 	[0xb8000+22*160+0x10],'M'
mov		byte 	[0xb8000+22*160+0x11],0x0c
mov		byte 	[0xb8000+22*160+0x12],'E'
mov		byte 	[0xb8000+22*160+0x13],0x0c
mov		byte 	[0xb8000+22*160+0x14],'D'
mov		byte 	[0xb8000+22*160+0x15],0x0c
mov		byte 	[0xb8000+22*160+0x16],'E'
mov		byte 	[0xb8000+22*160+0x17],0x0c
mov		byte 	[0xb8000+22*160+0x18],' '
mov		byte 	[0xb8000+22*160+0x19],0x0c
mov		byte 	[0xb8000+22*160+0x1a],'!'
mov		byte 	[0xb8000+22*160+0x1b],0x0c
mov		byte 	[0xb8000+22*160+0x1c],'!'
mov		byte 	[0xb8000+22*160+0x1d],0x0c
mov		byte 	[0xb8000+22*160+0x1e],'!'
mov		byte 	[0xb8000+22*160+0x1f],0x0c

mov		ax,00000000000_11_000B
mov		ss,ax
mov		esp,0x7c00

mov		ebp,esp
push	byte '#'

sub 	ebp,4

cmp		ebp,esp
jnz		over

pop		eax

mov		byte	[0xb8000+23*160+0x00],'S'
mov		byte	[0xb8000+23*160+0x01],0x0c
mov		byte	[0xb8000+23*160+0x02],'t'
mov		byte	[0xb8000+23*160+0x03],0x0c
mov		byte	[0xb8000+23*160+0x04],'a'
mov		byte	[0xb8000+23*160+0x05],0x0c
mov		byte	[0xb8000+23*160+0x06],'c'
mov		byte	[0xb8000+23*160+0x07],0x0c
mov		byte	[0xb8000+23*160+0x08],'k'
mov		byte	[0xb8000+23*160+0x09],0x0c
mov		byte	[0xb8000+23*160+0x0a],':'
mov		byte	[0xb8000+23*160+0x0b],0x0c
mov		byte	[0xb8000+23*160+0x0c],al
mov		byte	[0xb8000+23*160+0x0d],0x0c
mov		byte	[0xb8000+23*160+0x0e],','
mov		byte	[0xb8000+23*160+0x0f],0x0c
mov		byte	[0xb8000+23*160+0x10],'0'
mov		byte	[0xb8000+23*160+0x11],0x0c
mov		byte	[0xb8000+23*160+0x12],'K'
mov		byte	[0xb8000+23*160+0x13],0x0c
mov		byte	[0xb8000+23*160+0x14],'!'
mov		byte	[0xb8000+23*160+0x15],0x0c

over:
jmp $
