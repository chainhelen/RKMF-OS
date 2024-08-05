#include "textflag.h"

// 	参考 xv6 中 x86.h的实现
// 	static inline void
// 	outb(ushort port, uchar data)
// 	{
// 		asm volatile("out %0,%1" : : "a" (data), "d" (port));
// 	}
// 等价于
//
//        mov dx, word ptr [rsp + 16]
//        mov ax, word ptr [rsp + 8]
//        out dx, ax
//        ret
//
// 如下翻译成golang汇编

// Outb(port uint16, data byte)
TEXT ·Outb(SB), NOSPLIT, $0-3
	MOVW port+0(FP), DX
	MOVB data+2(FP), AX
	OUTB
    RET

// xv6
// static inline uchar
// inb(ushort port)
// {
//   uchar data;
//
//   asm volatile("in %1,%0" : "=a" (data) : "d" (port));
//   return data;
// }
// byte Inb(port uint16)
TEXT ·Inb(SB), NOSPLIT, $0-9
	MOVW port+0(FP), DX
	XORW AX, AX
	INB
	MOVB AX, ret+8(FP)
    RET

