org  0100h

BaseOfKernel	equ		8000h	; 	kernel.bin 	被加载到的 段地址
OffsetOfKernel	equ		0000h	;	kernel.bin	被加载到的 偏移地址
BaseOfKernelFilePhyAddr	equ	BaseOfKernel * 10h

BaseOfLoader			equ		9000h	; 	loader.bin 	被加载到的 段地址
OffsetOfLoader			equ		0100h	;	loader.bin	被加载到的 偏移地址
BaseOfLoaderFilePhyAddr	equ	BaseOfLoader * 10h 

KernelEntryPointPhyAddr	equ	030400h	; 注意：1、必须与 MAKEFILE 中参数 -Ttext 的值相等!!
BaseOfStack		equ		07c00h	; 	栈向地址减少的方向增长

; 变量
RootDirSectors			equ	14   	; 	根目录占用空间
SectorNoOfRootDirectory	equ	19		; 	Root	Directory 的第一扇区号	
SectorNoOfFAT1			equ	1		;	FAT1的第一个扇区号 = BPB_RsvdSecCnt
DeltaSectorNo			equ	17		;	DeltaSectorNo = BPB_RsvdSecCnt + (BPB_NumFATs * FATSz) - 2
									;	文件的开始Sector号 = DirEntry中的开始Sector号 + 根目录占用Sector数目 + DeltaSectorNo

jmp		LABEL_START

LABEL_START:
	mov	ax,	cs
	mov	ds,	ax
	mov	es,	ax
	mov	ss,	ax
	mov	sp,	BaseOfStack

	mov		dh,	0		;	"Loading "
	call	DispStr

	xor	ah,	ah		;	软驱复位
	xor	dl,	dl
	int	13h

;	下面在A盘的根目录寻找 loader.bin
	mov	word	[wSectorNo],	SectorNoOfRootDirectory
LABEL_SEARCH_IN_ROOT_DIR_BEGIN:
	cmp	word	[wRootDirSizeForLoop], 0
	jz	LABEL_NO_LOADERBIN	;	判断是否根目录已经读取完毕
	dec	word	[wRootDirSizeForLoop]
	mov	ax,	BaseOfKernel
	mov	es,	ax				;	es <- BaseOfKernel
	mov	bx,	OffsetOfKernel	;	bx	<-	OffsetOfKernel 于是，es:bx = BaseOfKernel
	mov	ax,	[wSectorNo]		;	ax <- Root Directory 中的某Sector	号
	mov	cl,	1
	call	ReadSector

	mov		si, KernelFileName	;	ds:si -> "Kernel bin"
	mov		di, OffsetOfKernel	;	es:di -> BaseOfKernel * 10h + OffsetOfKernel
	cld
	mov		dx, 10h
LABEL_SEARCH_FOR_LOADERBIN:
	cmp		dx,	0
	jz		LABEL_GOTO_NEXT_SECTOR_IN_ROOT_DIR
	dec		dx
	mov		cx, 11
LABEL_CMP_FILENAME:
	cmp		cx,	0
	jz		LABEL_FILENAME_FOUND
	dec	cx
	lodsb	;	ds:si -> al
	cmp		al, byte	[es:di]
	jz		LABEL_GO_ON
	jmp		LABEL_DIFFERENT	; 只要发现不一样的字符就表明本DirectoryEntry 不是我们要找的 loader.bin
LABEL_GO_ON:
	inc	di
	jmp	LABEL_CMP_FILENAME

LABEL_DIFFERENT:
	and	di, 0FFE0h
	add	di,	20h
	mov	si,	KernelFileName
	jmp	LABEL_SEARCH_FOR_LOADERBIN

LABEL_GOTO_NEXT_SECTOR_IN_ROOT_DIR:
	add	word [wSectorNo], 1
	jmp	LABEL_SEARCH_IN_ROOT_DIR_BEGIN

LABEL_NO_LOADERBIN:
	mov		dh, 2
	call	DispStr
	jmp	 $

LABEL_FILENAME_FOUND:
	mov		ax, RootDirSectors
	and		di,	0FFE0h			; di -> 当前条目的开始
	add		di,	01Ah			; di -> 首 Sector
	mov		cx,	word	[es:di]
	push	cx					;	保存此Sector 在FAT中的序号
	add		cx,	ax
	add		cx,	DeltaSectorNo	;	cl <- Kernel.bin 的其实扇区号 (0 - based)
	mov		ax,	BaseOfKernel
	mov		es,	ax				;	es <- BaseOfKernel
	mov		bx, OffsetOfKernel	;	bx <- OffsetOfKernel
	mov		ax, cx				;	ax <- Sector 号

LABEL_GOON_LOADING_FILE:
	push	ax
	push	bx
	mov		ah, 0Eh
	mov		al, '.'
	mov		bl, 0Fh
	int 	10h
	pop 	bx
	pop		ax

	mov		cl, 1
	call	ReadSector
	pop		ax
	call	GetFATEntry
	cmp		ax, 0FFFh
	jz		LABEL_FILE_LOADED
	push	ax
	mov		dx,	RootDirSectors
	add		ax, dx
	add		ax, DeltaSectorNo
	add		bx,	[BPB_BytsPerSec]
	jmp		LABEL_GOON_LOADING_FILE
LABEL_FILE_LOADED:
	mov		dh, 1
	call	DispStr

	jmp		preprotectmode

; -----------
; 函数名字 ReaderSector
;	作用：从 ax个Sector开始，将cl个Sector读入es:bx中
ReadSector:
	; -------------------
	; 怎样由扇区号求扇区在磁盘中的位置（扇区号 -> 柱面号，起始扇区，磁头号）
	; ---------------------
	; 设扇区号为 x
	;									 	柱面号  = y >> 1
	;		x                     商  y : 	磁头号 = y & 1
	; ---------------------- =>   余  z	   起始扇区号 = z + 1
	;	每磁道扇区数量
	push	bp
	mov		bp, sp
	sub		esp, 2
	
	mov		byte [bp-2],cl
	push	bx
	mov		bl, [BPB_SecPerTrk] ; bl : 除数
	div		bl					; y 在 al中， z在ah中
	inc		ah					; z++
	mov		cl, ah				; 	cl <- 起始扇区号
	mov		dh,	al				;	dh <- y
	shr		al, 1				;	y >> 1 (其实是 y/BPB_NumHeads, 这里BPB_NumHeads=2)
	mov		ch, al				; 	ch  <- 柱面号
	and		dh, 1				; 	dh & 1 = 磁头号
	pop		bx					; 	恢复 bx
 	; 至此，"柱面号，起始扇区，磁头号" 全部得到
	mov		dl, [BS_DrvNum]		;	驱动器号 (0 代表 A 盘)
.GoOnReading:
	mov		ah, 2				;	读
	mov		al, byte [bp-2]		;	读 al 个扇区
	int		13h
	jc		.GoOnReading		;	如果读取错误 CF 会被置为 1，这时就不停地读，直到正确为止

	add		esp, 2
	pop		bp

	ret

MessageLength	equ	9
BootMessage:	db	"Loading  "
Message1		db	"Ready.   "
Message2		db	"No LOADER"

; -----------
; 函数名：DispStr
; ----------------------
; 作用:
;		显示一个字符串，函数开始时 dh 中应该是字符串序号(0-based)
DispStr:
	mov		ax,	MessageLength
	mul		dh
	add		ax, BootMessage
	mov		bp,	ax
	mov		ax, ds
	mov		es,	ax
	mov		cx,	MessageLength
	mov		ax, 01301h
	mov		bx,	0007h
	add		dh, 2
	mov		dl, 0
	int		10h
	ret

;----------------------------------------------------------------------------
; 函数名: GetFATEntry
;----------------------------------------------------------------------------
; 作用:
;	找到序号为 ax 的 Sector 在 FAT 中的条目, 结果放在 ax 中
;	需要注意的是, 中间需要读 FAT 的扇区到 es:bx 处, 所以函数一开始保存了 es 和 bx
GetFATEntry:
	push	es
	push	bx
	push	ax
	mov	ax, BaseOfKernel; `.
	sub	ax, 0100h	;  | 在 BaseOfKernel 后面留出 4K 空间用于存放 FAT
	mov	es, ax		; /
	pop	ax
	mov	byte [bOdd], 0
	mov	bx, 3
	mul	bx			; dx:ax = ax * 3
	mov	bx, 2
	div	bx			; dx:ax / 2  ==>  ax <- 商, dx <- 余数
	cmp	dx, 0
	jz	LABEL_EVEN
	mov	byte [bOdd], 1
LABEL_EVEN:;偶数
	; 现在 ax 中是 FATEntry 在 FAT 中的偏移量,下面来
	; 计算 FATEntry 在哪个扇区中(FAT占用不止一个扇区)
	xor	dx, dx			
	mov	bx, [BPB_BytsPerSec]
	div	bx ; dx:ax / BPB_BytsPerSec
		   ;  ax <- 商 (FATEntry 所在的扇区相对于 FAT 的扇区号)
		   ;  dx <- 余数 (FATEntry 在扇区内的偏移)。
	push	dx
	mov	bx, 0 ; bx <- 0 于是, es:bx = (BaseOfKernel - 100):00
	add	ax, SectorNoOfFAT1 ; 此句之后的 ax 就是 FATEntry 所在的扇区号
	mov	cl, 2
	call	ReadSector ; 读取 FATEntry 所在的扇区, 一次读两个, 避免在边界
			   ; 发生错误, 因为一个 FATEntry 可能跨越两个扇区
	pop	dx
	add	bx, dx
	mov	ax, [es:bx]
	cmp	byte [bOdd], 1
	jnz	LABEL_EVEN_2
	shr	ax, 4
LABEL_EVEN_2:
	and	ax, 0FFFh

LABEL_GET_FAT_ENRY_OK:

	pop	bx
	pop	es
	ret


dptseg			equ		7e0h
gdt_size		dw		64-1
gdt_base		dd		0x00007e00
;----------------------------------------------------------------------------
; 函数名: ConstructGdt
;----------------------------------------------------------------------------
; 构造GDT表
ConstructGdt:
	mov		ax,dptseg
	mov		es,ax
	lgdt	[gdt_size]
	; 创建 #0 空描述符
	mov	dword	[es:0x00],0x00
	mov	dword	[es:0x04],0x00

	; 创建 #1 描述符，代码段CS
	mov	dword	[es:0x08],0x0000ffff
	mov	dword	[es:0x0c],0x00cf9800
	
	; #2描述符，保护模式下的数据段描述符：DS（文本模式下的显示缓冲区） 10
	mov	dword	[es:0x10],0x0000ffff
	mov	dword	[es:0x14],0x00cf9200
	
	; #3描述符
	mov	word	[es:0x18],0xffff ;显卡线性地址的低16位
	mov	byte	[es:0x1a],bl
	mov	byte	[es:0x1b],bh
	
	shr ebx,16
	mov	byte	[es:0x1c],bl;显卡的高16位地
	mov	word	[es:0x1d],0x4f92
	mov	byte	[es:0x1f],bh
	
	; #4 DS (用户设置的分辨率)
	mov	dword	[es:0x20],0x7d000100
	mov	dword	[es:0x24],0x00409200
	
	; #5 堆栈描述符：SS
	mov	dword	[es:0x28],0x00007a00
	mov	dword	[es:0x2c],0x00409600

	; #6
	mov	dword	[es:0x34],0x0000ffff
	mov	dword	[es:0x3C],0x00cf9200

	; #7
	mov	dword	[es:0x44],0x0000ffff
	mov	dword	[es:0x4C],0x00cf9200
	ret

Setmode:
	mov ah,0h
	mov al,13h         ;320*200
	int 10h
    ret

BOOTBASE   	EQU         0x6000  
SCRNX		EQU			BOOTBASE    	;分辨率X, 1024
SCRNY		EQU			BOOTBASE+2     	;分辨率Y, 768
VRAM		EQU			BOOTBASE+4     	;存放各种显示模式下的显存地址

Setmode2:
	mov ax,4f02h 		;设置图形模式：1024×768 256色
	mov bx,4105h
	int 10h
	
	push	es
	mov		ax, 0
	mov		es, ax
	; 获取高清显卡地址
	mov di,	0x6000 ;	BIOS中断获取显卡线性地址,暂存0x6000+40处
	mov ax,	0x4f01
	mov cx,	0x101
	int    	0x10

	;显卡线性地址：返回结构体中偏移量40的地方，即es:di+40处，用4字节 ;dw[ es:di+40 ]低位,[ es:di+42 ]高位，一般是: 0xe000_0000
	mov ebx,		[es:di+40]      ;es=0,di=0x6000
	mov [es:VRAM],	ebx           	;最终存在[VRAM],后面创建GDT也需要使用
	
	mov	ax, 1024
	mov	[es:SCRNX], ax
	mov	ax, 768
	mov	[es:SCRNY], ax

	pop		es
	
	ret

preprotectmode:
	; 设置调色板
	call 	Setmode2
	call	ConstructGdt
	; 打开地址线A20
	in 	al,0x92
	or	al,0000_0010B
	out	0x92,al

	; 关中断
	cli

	; 切换保护模式
	mov	eax,cr0
	or	eax,1
	mov	cr0,eax

	jmp	dword	0x0008:(BaseOfLoaderFilePhyAddr + inprotectmode)


wRootDirSizeForLoop 	dw 	RootDirSectors
wSectorNo				dw	0
bOdd					db	0
KernelFileName			db	"KERNEL  BIN", 0


ALIGN 32
[BITS 32]
inprotectmode:
	; 数据段选择子
	mov		ax, 	0x10
	mov		ds, 	ax
	mov		es, 	ax
	mov		ss,		ax
	mov		fs,		ax
    ; 显卡选择子
	mov		ax, 	0x18
	mov		gs,		ax
	; 栈底位置
	mov		esp,	BaseOfStack

	call	InitKernel
	
	jmp		0x0008:KernelEntryPointPhyAddr

; InitKernel ---------------------------------------------------------------------------------
; 将 KERNEL.BIN 的内容经过整理对齐后放到新的位置
; 遍历每一个 Program Header，根据 Program Header 中的信息来确定把什么放进内存，放到什么位置，以及放多少。
; --------------------------------------------------------------------------------------------
InitKernel:
        xor   esi, esi
        mov   cx, word [BaseOfKernelFilePhyAddr+2Ch];`. ecx <- pELFHdr->e_phnum
        movzx ecx, cx                               ;/
        mov   esi, [BaseOfKernelFilePhyAddr + 1Ch]  ; esi <- pELFHdr->e_phoff
        add   esi, BaseOfKernelFilePhyAddr;esi<-OffsetOfKernel+pELFHdr->e_phoff
.Begin:
        mov   eax, [esi + 0]
        cmp   eax, 0                      	; PT_NULL
        jz    .NoAction
        push  dword [esi + 010h]    		;size ;`.
        mov   eax, [esi + 04h]            	; |
        add   eax, BaseOfKernelFilePhyAddr	; | memcpy((void*)(pPHdr->p_vaddr),
        push  eax		    				;src  ; |      uchCode + pPHdr->p_offset,
        push  dword [esi + 08h]     		;dst  ; |      pPHdr->p_filesz;
        call  MemCpy                      	; |
        add   esp, 12                     	;/
.NoAction:
        add   esi, 020h                   	; esi += pELFHdr->e_phentsize
        dec   ecx
        jnz   .Begin

        ret
; InitKernel ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
; ------------------------------------------------------------------------
; 内存拷贝，仿 memcpy
; ------------------------------------------------------------------------
; void* MemCpy(void* es:pDest, void* ds:pSrc, int iSize);
; ------------------------------------------------------------------------
MemCpy:
	push	ebp
	mov	ebp, esp

	push	esi
	push	edi
	push	ecx

	mov	edi, [ebp + 8]	; Destination
	mov	esi, [ebp + 12]	; Source
	mov	ecx, [ebp + 16]	; Counter
.1:
	cmp	ecx, 0		; 判断计数器
	jz	.2		; 计数器为零时跳出

	mov	al, [ds:esi]		; ┓
	inc	esi			; ┃
					; ┣ 逐字节移动
	mov	byte [es:edi], al	; ┃
	inc	edi			; ┛

	dec	ecx		; 计数器减一
	jmp	.1		; 循环
.2:
	mov	eax, [ebp + 8]	; 返回值

	pop	ecx
	pop	edi
	pop	esi
	mov	esp, ebp
	pop	ebp

	ret			; 函数结束，返回
; MemCpy 结束-------------------------------------------------------------


; FAT 格式
BS_OEMName		DB	'ForrestY'		; 	OEM string 8个字符
BPB_BytsPerSec	DW	512				;	每扇区字节数
BPB_SecPerClus	DB	1				;	每簇多少扇区
BPB_RsvdSecCnt	DW	1				;	Boot 记录占用多少扇区
BPB_NumFATs		DB	2				;	共有多少个FAT表
BPB_RootEntCnt	DW	224				;	根目录文件最大数
BPB_TotSec16	DW	2880			;	逻辑扇区总数
BPB_Media		DB	0XF0			;	媒体描述符
BPB_FATSz16		DW	9				;	每FAT扇区数
BPB_SecPerTrk	DW	18				;	每磁道扇区数
BPB_NumHeads	DW	2				;	磁头数
BPB_HiddSec		DD	0				;	隐藏扇区数
BPB_TotSec32	DD	0				;	wTotalSectorCount为0时这个值记录扇区数
BS_DrvNum		DB	0				;	中断13的驱动器号
BS_Reserved1	DB	0				;	未使用
BS_BootSig		DB	29h				;	扩展引导标记(29h)
BS_Volld		DD	0				;	卷序列号
BS_VolLab		DB	'RKMF_OS_.02'	;	卷标，必须11个字节
BS_FileSysType	DB	'FAT12   '		;	文件系统类型，必须8个字节
