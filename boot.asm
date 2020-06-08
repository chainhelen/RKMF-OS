mbrseg 			equ			7c0h
outseg			equ			800h

jmp start
newwelcome:		db	'-----------------Welcome ch os------------', '$'
message1:		db	'step now is in mbr', '$'
message2:		db	'step2: now jmp out mbr', '$'
message3:		db	'memory address is --------', '$'
message4:		db	'CS:????', '$'

start:
mov		ax, mbrseg
mov		ds, ax
call	welcome
call	newline
call	newline
call	inmbr
call	showcs
call	newline

jmp 	outseg:0

call	outmbr
call	showcs

welcome:
mov		si,newwelcome
call	printstr
ret

inmbr:
mov		si,message1
call	printstr
call	newline
mov		si,message3
call	printstr
ret

outmbr:
mov		si,message2
call	printstr
call	newline
mov		si,message3
call	printstr
ret

printstr:
	mov	al,[si]
	cmp al,'$'
	je	discover
	mov ah, 0eh
	int 10h
	inc si
	jmp printstr
discover:
	ret

newline:
	mov ah, 0eh
	mov	al, 0dh
	int	10h
	mov	al, 0ah
	int 10h
	ret

showcs:
	mov		ax,cs
	
	mov		dl,ah
	call	HL4BIT
	mov		dl,bh
	call	ASCII
	mov		[message4+3],dl

	mov		dl,bl
	call	ASCII
	mov		[message4+4],dl

	mov		dl,al
	call	HL4BIT
	mov		dl,bh
	call	ASCII
	mov		[message4+5],dl
	
	mov		dl,bl
	call	ASCII
	mov		[message4+6],dl
	
	mov		si,message4
	call	printstr

	ret

;----------------
ASCII: 	cmp dl,9
		jg	LETTER
		add dl,30H
		RET
LETTER:	add dl,37h
		RET

;-----------
HL4BIT:	mov	dh,dl
		mov	bl,dl
		shr dh,1
		shr dh,1
		shr dh,1
		shr dh,1
		mov	bh,dh
		add bl,0fh
		ret

times 510-($-$$) db 0
db 0x55,0xaa
