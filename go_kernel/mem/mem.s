#include "textflag.h"

TEXT ·lcr3(SB), NOSPLIT, $0-8
    MOVQ e+0(FP), AX
    MOVQ AX, CR3
    RET

TEXT ·pageEnable(SB), NOSPLIT, $0-0
	MOVQ CR4, AX
	BTSQ $5, AX
	MOVQ AX, CR4

	MOVQ CR0, AX
	BTSQ $31, AX
	MOVQ AX, CR0
	RET

