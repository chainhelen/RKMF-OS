[section .text]	; 代码在此
global 	_start		; 	导出 	_start，给链接器使用
global	out_byte
global	in_byte
global  divide_error;
global  single_step_exception;
global  nmi;
global  breakpoint_exception;
global  overflow;
global  bounds_check;
global  inval_opcode;
global  copr_not_available;
global  double_fault;
global  copr_seg_overrun;
global  inval_tss;
global  segment_not_present;
global  stack_exception;
global  general_protection;
global  page_fault;
global  copr_error;
; global  hwint00;
global 	asm_timer_handler;
; global  hwint01;
global	asm_keyboardhandler;
global  hwint02;
global  hwint03;
global  hwint04;
global  hwint05;
global  hwint06;
global  hwint07;
global  hwint08;
global  hwint09;
global  hwint10;
global  hwint11;
; global  hwint12;
global  asm_mousehandler
global  hwint13
global  hwint14
global  hwint15
global 	io_hlt
global	io_cli
global	io_sti
global  io_stihlt
global  io_loop
global	load_idt_reg
global 	asm_memtest
global  io_load_eflags
global  io_store_eflags
global 	load_tr
global 	farjmp


extern	spurious_irq
extern 	cstart	;	导入	cstart
extern	idt_ptr	;	导入	idt的指针
extern 	keyboardhandler;
extern  mousehandler
extern  timer_handler

_start:	; 跳到这里来的时候，我们假设 gs 指向显存
	; 	mov ax, ds 这行指令没什么用处，只是为了调试时候看到对应指令
	mov		ax, ds
	call 	cstart
	jmp		$
	mov		ax, ds

; ========================================================================
;                  void out_byte(u16 port, u8 value);
; ========================================================================
out_byte:
	mov	edx, [esp + 4]		; port
	mov	al, [esp + 4 + 4]	; value
	out	dx, al
	ret

; ========================================================================
;                  u8 in_byte(u16 port);
; ========================================================================
in_byte:
	mov	edx, [esp + 4]		; port
	xor	eax, eax
	in	al, dx
	ret

; 中断和异常 -- 硬件中断
; ---------------------------------
%macro  hwint_master    1
        push    %1
        call    spurious_irq
        add     esp, 4
        hlt
%endmacro
; ---------------------------------
; ---------------------------------
%macro  hwint_slave     1
        push    %1
        call    spurious_irq
        add     esp, 4
        hlt
%endmacro
; ---------------------------------



; 中断和异常 -- 硬件中断
ALIGN   16
asm_timer_handler:                ; Interrupt routine for irq 0 (the clock).
	cli	; 应禁止中断
	CALL	timer_handler
	IRET

; ALIGN   16
; hwint01:                ; Interrupt routine for irq 1 (keyboard)
;        hwint_master    1

KeyboardIOInterruptMsgLength	equ		23
KeyboardIOInterruptMsg			db 		"keyboard io interrupt: "

ALIGN   16
asm_keyboardhandler:
	cli ;应禁止中断
	; mov		ax, 000
	CALL	keyboardhandler
	; mov     al , 0x20  ;告诉硬件,中断处理完毕,即发送 EOI 消息
	; out     0x20 , al
	; out     0xa0 , al
	IRET

ALIGN	16
asm_mousehandler:
	cli	; 应禁止中断
	CALL	mousehandler
	IRET


ALIGN   16
hwint02:                ; Interrupt routine for irq 2 (cascade!)
        hwint_master    2

ALIGN   16
hwint03:                ; Interrupt routine for irq 3 (second serial)
        hwint_master    3

ALIGN   16
hwint04:                ; Interrupt routine for irq 4 (first serial)
        hwint_master    4

ALIGN   16
hwint05:                ; Interrupt routine for irq 5 (XT winchester)
        hwint_master    5

ALIGN   16
hwint06:                ; Interrupt routine for irq 6 (floppy)
        hwint_master    6

ALIGN   16
hwint07:                ; Interrupt routine for irq 7 (printer)
        hwint_master    7

ALIGN   16
hwint08:                ; Interrupt routine for irq 8 (realtime clock).
        hwint_slave     8

ALIGN   16
hwint09:                ; Interrupt routine for irq 9 (irq 2 redirected)
        hwint_slave     9

ALIGN   16
hwint10:                ; Interrupt routine for irq 10
        hwint_slave     10

ALIGN   16
hwint11:                ; Interrupt routine for irq 11
        hwint_slave     11
ALIGN   16
hwint12:                ; Interrupt routine for irq 12
        hwint_slave     12

ALIGN   16
hwint13:                ; Interrupt routine for irq 13 (FPU exception)
        hwint_slave     13

ALIGN   16
hwint14:                ; Interrupt routine for irq 14 (AT winchester)
        hwint_slave     14

ALIGN   16
hwint15:                ; Interrupt routine for irq 15
        hwint_slave     15


; 中断和异常 -- 异常
divide_error:
	; mov byte [0xb8000+23*160+0x00],'d'
	; mov byte [0xb8000+23*160+0x01],0x0c
	; mov byte [0xb8000+23*160+0x02],'i'
	; mov byte [0xb8000+23*160+0x03],0x0c
	; mov byte [0xb8000+23*160+0x04],'v'
	; mov byte [0xb8000+23*160+0x05],0x0c
	; mov byte [0xb8000+23*160+0x06],'e'
	; mov byte [0xb8000+23*160+0x07],0x0c
	; mov byte [0xb8000+23*160+0x08],'r'
	; mov byte [0xb8000+23*160+0x09],0x0c
	; mov byte [0xb8000+23*160+0x0a],'r'
	; mov byte [0xb8000+23*160+0x0b],0x0c
	; mov byte [0xb8000+23*160+0x0c],'o'
	; mov byte [0xb8000+23*160+0x0d],0x0c
	; mov byte [0xb8000+23*160+0x0e],'r'
	; mov byte [0xb8000+23*160+0x0f],0x0c
	; IRET
	hlt
single_step_exception:
	hlt
nmi:
	hlt
breakpoint_exception:
	hlt
overflow:
	hlt
bounds_check:
	hlt
inval_opcode:
	hlt
copr_not_available:
	hlt
double_fault:
	hlt
copr_seg_overrun:
	hlt
inval_tss:
	hlt
segment_not_present:
	hlt
stack_exception:
	hlt
general_protection:
	hlt
page_fault:
	hlt
copr_error:
	hlt

io_hlt:
		HLT
		RET

io_cli:
		CLI
		RET

io_sti:
		STI
		RET

io_stihlt:
		STI
		HLT
		RET

io_loop:
	jmp		$
	RET

load_idt_reg:
	lidt	[idt_ptr]
	RET


asm_memtest:
    PUSH	EDI						; EBX, ESI, EDI
    PUSH	ESI
    PUSH	EBX
    MOV		ESI,0xaa55aa55			; pat0 = 0xaa55aa55;
    MOV		EDI,0x55aa55aa			; pat1 = 0x55aa55aa;
    MOV		EAX,[ESP+12+4]			; i = start;
mts_loop:
    MOV		EBX,EAX
    ADD		EBX,0xffc				; p = i + 0xffc;
    MOV		EDX,[EBX]				; old = *p;
    MOV		[EBX],ESI				; *p = pat0;
    XOR		DWORD [EBX],0xffffffff	; *p ^= 0xffffffff;
    CMP		EDI,[EBX]				; if (*p != pat1) goto fin;
    JNE		mts_fin
    XOR		DWORD [EBX],0xffffffff	; *p ^= 0xffffffff;
    CMP		ESI,[EBX]				; if (*p != pat0) goto fin;
    JNE		mts_fin
    MOV		[EBX],EDX				; *p = old;
    ADD		EAX,0x1000				; i += 0x1000;
    CMP		EAX,[ESP+12+8]			; if (i <= end) goto mts_loop;
    JBE		mts_loop
    POP		EBX
    POP		ESI
    POP		EDI
    RET
mts_fin:
    MOV		[EBX],EDX				; *p = old;
    POP		EBX
    POP		ESI
    POP		EDI
    RET

io_load_eflags:
		PUSHFD		; PUSH EFLAGS
		POP		EAX
		RET

io_store_eflags:
		MOV		EAX,[ESP+4]
		PUSH	EAX
		POPFD		; POP EFLAGS
		RET
load_tr:		; void load_tr(int tr);
		LTR		[ESP+4]			; tr
		RET
farjmp:		; void farjmp(int eip, int cs);
		JMP		FAR	[ESP+4]				; eip, cs
		RET
