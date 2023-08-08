#include "textflag.h"

TEXT ·lgdt(SB), NOSPLIT, $0-8
    MOVQ gdtptr+0(FP), AX
    LGDT (AX)
    RET
