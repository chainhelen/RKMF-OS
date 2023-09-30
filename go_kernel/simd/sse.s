#include "textflag.h"

// sseInit initializes the SSE instruction set.
// https://wiki.osdev.org/SSE 代码相关来源
// clear the CR0.EM bit (bit 2) [ CR0 &= ~(1 << 2) ]
// set the CR0.MP bit (bit 1) [ CR0 |= (1 << 1) ]
// set the CR4.OSFXSR bit (bit 9) [ CR4 |= (1 << 9) ]
// set the CR4.OSXMMEXCPT bit (bit 10) [ CR4 |= (1 << 10) ]
TEXT ·sseInit(SB), NOSPLIT, $0
	MOVL CR0, AX
	ANDW $0xFFFB, AX
	ORW  $0x2, AX
	MOVL AX, CR0
	MOVL CR4, AX
	ORW  $3<<9, AX
	MOVL AX, CR4
	RET

// func checkSSE() (ok bool)，注意8字节对齐
TEXT ·checkSSE(SB), NOSPLIT, $0-8
    MOVL $0x1, AX
    CPUID
    TESTL DX, 1<< 25
    JNZ  nosse
    TESTL DX, 1<< 26
    JNZ nosse
    MOVL $1, ok+0(FP)
    RET
nosse:
    MOVL $0, ok+0(FP)
    RET
