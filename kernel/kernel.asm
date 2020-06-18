[section .text]	; 代码在此
global 	_start		; 	导出 	_start，给链接器使用

extern 	cstart	;	导入	cstart

_start:	; 跳到这里来的时候，我们假设 gs 指向显存
	; mov	ah, 0Fh				; 0000: 黑底    1111: 白字
	; mov	al, 'K'
	; mov	[gs:((80 * 1 + 39) * 2)], ax	; 屏幕第 1 行, 第 39 列。
	mov		ax, ds
	call 	cstart
	jmp	$

