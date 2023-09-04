#include "textflag.h"

TEXT ·rt0(SB), NOSPLIT, $0-0
	// switch to new stack
	MOVQ $0x80000, SP
	XORQ BP, BP

	// DI and SI store multiboot magic and info passed by bootloader
	SUBQ $0x10, SP
	MOVQ DI, 0(SP)
	MOVQ SI, 8(SP)
	CALL ·preinit(SB)
 	INT  $3
