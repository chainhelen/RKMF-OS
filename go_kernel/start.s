#include "textflag.h"

TEXT ·start(SB), NOSPLIT, $0-0
    MOVQ $0x80000, SP
    XORQ BP, BP

    CALL ·preinit(SB)
    INT $3
