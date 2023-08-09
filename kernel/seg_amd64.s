#include "textflag.h"

TEXT ·lgdt(SB), NOSPLIT, $0-8
    MOVQ gdtptr+0(FP), AX
    LGDT (AX)
    RET


TEXT ·ltr(SB), NOSPLIT, $0-8
    MOVQ sel+0(FP), AX
    LTR AX
    RET


// lidt(idtptr uint64) - Load Interrupt Descriptor Table Register.
TEXT ·lidt(SB), NOSPLIT, $0-8
	MOVQ idtptr+0(FP), AX
	LIDT (AX)
	RET
