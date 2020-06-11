%include    "const.inc"

NUMsector		EQU		3
NUMheader		EQU		0
NUMcylind		EQU		0

jmp start
msgwelcome:			db	'--Welcome RKMF OS--','$'
msgstep1:			db	'step1: now is in mbr','$'
msgmem1:			db	'memory addresss is---','$'
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
ret

inmbrshow:
mov		si,msgwelcome
call	printstr
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
