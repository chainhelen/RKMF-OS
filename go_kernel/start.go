package go_kernel

import "github.com/chainhelen/RKMF-OS/go_kernel/mem"

func preinit(magic, mbiptr uintptr) {
	gdtInit()
	idtInit()
	mem.KM.FreeRange()
	mem.KM.SetUpKvm()
}
