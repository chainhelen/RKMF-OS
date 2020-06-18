[section .text]	; 代码在此
global 	_start		; 	导出 	_start，给链接器使用
global  io_in8		;   导出    给调色板程序使用
global  io_in16		;   导出    给调色板程序使用
global  io_in32		;   导出    给调色板程序使用
global  io_out8		;   导出    给调色板程序使用
global  io_out16	;   导出    给调色板程序使用
global  io_out32	;   导出    给调色板程序使用
global  io_cli
global  io_load_eflags
global  io_store_eflags

extern 	cstart	;	导入	cstart

_start:	; 跳到这里来的时候，我们假设 gs 指向显存
	; mov	ah, 0Fh				; 0000: 黑底    1111: 白字
	; mov	al, 'K'
	; mov	[gs:((80 * 1 + 39) * 2)], ax	; 屏幕第 1 行, 第 39 列。
	mov		ax, ds
	call 	cstart
	jmp	$

io_in8:
    mov  edx, [esp + 4]
    mov  eax, 0
    in   al, dx

io_in16:
    mov  edx, [esp + 4]
    mov  eax, 0
    in   ax, dx

io_in32:
    mov edx, [esp + 4]
    in  eax, dx
    ret

io_out8:
    mov edx, [esp + 4]
    mov al, [esp + 8]
    out dx, al
    ret

io_out16:
    mov edx, [esp + 4]
    mov eax, [esp + 8]
    out dx, ax
    ret

io_out32:
    mov edx, [esp + 4]
    mov eax, [esp + 8]
    out dx, eax
    ret

io_cli:
    CLI
    RET

io_load_eflags:
    pushfd
    pop  eax
    ret

io_store_eflags:
    mov eax, [esp + 4]
    push eax
    popfd
    ret
