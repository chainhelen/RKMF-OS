NUMsector		EQU		3
NUMheader		EQU		0
NUMcylind		EQU		0

mbrseg			equ		7c0h
newseg			equ		800h
dptseg			equ		7e0h

jmp start
msgwelcome:			db	'--Welcome RKMF OS--','$'
msgstep1:			db	'step1: now is in mbr','$'
msgmem1:			db	'memory addresss is------','$'
msgcs1:				db	'cs:????H','$'

cylind		db	'cylind:?? $',0
header		db	'header:?? $',0
sector		db	'sector:?? $',2
floppyok	db	'read ok','$'
fyerror		db	'read error','$'

start:
call 	inmbr
call	floppyload
jmp		newseg:0

inmbr:
mov		ax, mbrseg
mov		ds,	ax
mov		ax, newseg
mov		es, ax
call	inmbrshow
call	showcs
call	newline
call	newline
call	newline
ret

inmbrshow:
mov		si,msgwelcome
call	printstr
call 	newline
call 	newline
mov		si,msgstep1
call	printstr
call	newline
mov		si,msgmem1
call	printstr
ret

printstr:
	mov	al,[si]
	cmp	al,'$'
	je	disover
	mov	ah,0eh
	int 10h
	inc	si
	jmp	printstr
disover:
	ret

newline:
	mov	al,0dh
	mov	ah,0eh
	int	10h
	mov	al,0ah
	int 10h
	ret

showcs:
	mov		ax,cs
	mov		dl,ah
	call	hl4bit

	mov		dl,bh
	call	ascii
	mov		[msgcs1+3],dl

	mov		dl,bl
	call	ascii
	mov		[msgcs1+4],dl

	mov		dl,al
	call	hl4bit

	mov		dl,bh
	call	ascii
	mov		[msgcs1+5],dl

	mov		dl,bl
	call	ascii
	mov		[msgcs1+6],dl

	mov		si, msgcs1
	call	printstr

	ret

ascii:
	cmp		dl,9
	jg		letter
	add		dl,30h
	ret
letter:
	add		dl,37h
	ret

hl4bit:
	mov		dh,dl
	mov		bl,dl
	shr		dh,1
	shr		dh,1
	shr		dh,1
	shr		dh,1
	mov		bh,dh
	and		bl,0fh
	ret

floppyload:
	call	read1sector
	mov		ax,es
	add		ax,0x0020
	mov		es,ax
	
	inc		byte 	[sector+11]
	cmp		byte	[sector+11],NUMsector+1
	jne		floppyload
	mov		byte	[sector+11],1
	inc		byte	[header+11]
	cmp		byte	[header+11],NUMheader+1
	jne		floppyload
	mov		byte	[header+11],0
	inc		byte	[cylind+11]
	cmp		byte	[cylind+11],NUMcylind+1
	jne		floppyload

	ret

numtoascii:
	mov		ax,0
	mov		al,cl
	mov		bl,10
	div		bl
	add		ax,3030h
	ret

readinfo:
	mov		si,cylind
	call	printstr
	mov		si,header
	call	printstr
	mov		si,sector
	call	printstr
	ret

read1sector:
	mov		cl,[sector+11]
	call	numtoascii
	mov		[sector+7],al
	mov		[sector+8],ah

	mov		cl,[header+11]
	call	numtoascii
	mov		[header+7],al
	mov		[header+8],ah

	mov		cl,[cylind+11]
	call	numtoascii
	mov		[cylind+7],al
	mov		[cylind+8],ah

	mov		ch,[cylind+11]
	mov		dh,[header+11]
	mov		cl,[sector+11]

	call	readinfo
	mov		di,0
retry:
	mov		ah,02h
	mov		al,1
	mov		bx,0
	mov		dl,00h
	int		13h
	jnc		readok
	inc		di
	mov		ah,0x00
	mov		dl,0x00
	int		0x13
	cmp		di,5
	jne		retry

	mov		si,fyerror
	call	printstr
	call	newline
	jmp		exitread
readok:
	mov		si,floppyok
	call	printstr
	call	newline
exitread:
	ret

times 510-($-$$) db 0
db 0x55,0xaa

;
;
;

jmp		newprogram

gdt_size dw 32-1   ; GDT 表大小
gdt_base dd 0x00007e00 ;GDT的物理地址

msgstep2:			db	'step2: now jmp out mbr','$'
mesmem2:			db	'will visit memory address is---','$'
msgcs2:				db	'cs:????H','$'

msgstep3:			db	'Step3:now enter protect mode', '$'

newprogram:
mov		ax,newseg
sub		ax,20h
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
call	newlinenew
mov		si,msgstep2
call	printstrnew
call	newlinenew
mov		si,mesmem2
call	printstrnew
ret

showprotect:
call	newlinenew
call	newlinenew
mov		si,msgstep3
call	newlinenew
call	printstrnew
call	newlinenew
ret

showcsnew:
	mov		ax,cs
	mov		dl,ah
	call	HL4BITnew
	mov		dl,bh
	call	asciinew
	mov		[msgcs2+3],dl

	mov		dl,bl
	call	asciinew
	mov		[msgcs2+4],dl

	mov		dl,al
	call	HL4BITnew
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
	ret
letternew:
	add		dl,37h
	ret

HL4BITnew:
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

jmp dword	0x0008:inprotectmode-512

[bits 32]
inprotectmode:
mov		ax,00000000000_10_000B
mov		ds,ax

mov		byte 	[0xb8000+20*160+0x00],'P'
mov		byte 	[0xb8000+20*160+0x01],0x0c
mov		byte 	[0xb8000+20*160+0x02],'R'
mov		byte 	[0xb8000+20*160+0x03],0x0c
mov		byte 	[0xb8000+20*160+0x04],'O'
mov		byte 	[0xb8000+20*160+0x05],0x0c
mov		byte 	[0xb8000+20*160+0x06],'T'
mov		byte 	[0xb8000+20*160+0x07],0x0c
mov		byte 	[0xb8000+20*160+0x08],'E'
mov		byte 	[0xb8000+20*160+0x09],0x0c
mov		byte 	[0xb8000+20*160+0x0a],'C'
mov		byte 	[0xb8000+20*160+0x0b],0x0c
mov		byte 	[0xb8000+20*160+0x0c],'T'
mov		byte 	[0xb8000+20*160+0x0d],0x0c
mov		byte 	[0xb8000+20*160+0x0e],'-'
mov		byte 	[0xb8000+20*160+0x0f],0x0c
mov		byte 	[0xb8000+20*160+0x10],'M'
mov		byte 	[0xb8000+20*160+0x11],0x0c
mov		byte 	[0xb8000+20*160+0x12],'E'
mov		byte 	[0xb8000+20*160+0x13],0x0c
mov		byte 	[0xb8000+20*160+0x14],'D'
mov		byte 	[0xb8000+20*160+0x15],0x0c
mov		byte 	[0xb8000+20*160+0x16],'E'
mov		byte 	[0xb8000+20*160+0x17],0x0c
mov		byte 	[0xb8000+20*160+0x18],' '
mov		byte 	[0xb8000+20*160+0x19],0x0c
mov		byte 	[0xb8000+20*160+0x1a],'!'
mov		byte 	[0xb8000+20*160+0x1b],0x0c
mov		byte 	[0xb8000+20*160+0x1c],'!'
mov		byte 	[0xb8000+20*160+0x1d],0x0c
mov		byte 	[0xb8000+20*160+0x1e],'!'
mov		byte 	[0xb8000+20*160+0x1f],0x0c

mov		ax,00000000000_11_000B
mov		ss,ax
mov		esp,0x7c00

mov		ebp,esp
push	byte '#'

sub 	ebp,4

cmp		ebp,esp
jnz		over

pop		eax

mov		byte	[0xb8000+22*160+0x00],'S'
mov		byte	[0xb8000+22*160+0x01],0x0c
mov		byte	[0xb8000+22*160+0x02],'t'
mov		byte	[0xb8000+22*160+0x03],0x0c
mov		byte	[0xb8000+22*160+0x04],'a'
mov		byte	[0xb8000+22*160+0x05],0x0c
mov		byte	[0xb8000+22*160+0x06],'c'
mov		byte	[0xb8000+22*160+0x07],0x0c
mov		byte	[0xb8000+22*160+0x08],'k'
mov		byte	[0xb8000+22*160+0x09],0x0c
mov		byte	[0xb8000+22*160+0x0a],':'
mov		byte	[0xb8000+22*160+0x0b],0x0c
mov		byte	[0xb8000+22*160+0x0c],al
mov		byte	[0xb8000+22*160+0x0d],0x0c
mov		byte	[0xb8000+22*160+0x0e],','
mov		byte	[0xb8000+22*160+0x0f],0x0c
mov		byte	[0xb8000+22*160+0x10],'0'
mov		byte	[0xb8000+22*160+0x11],0x0c
mov		byte	[0xb8000+22*160+0x12],'K'
mov		byte	[0xb8000+22*160+0x13],0x0c
mov		byte	[0xb8000+22*160+0x14],'!'
mov		byte	[0xb8000+22*160+0x15],0x0c

over:
jmp $
