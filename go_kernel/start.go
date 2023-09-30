package go_kernel

import (
	"github.com/chainhelen/RKMF-OS/go_kernel/mem"
	"github.com/chainhelen/RKMF-OS/go_kernel/simd"
	"github.com/chainhelen/RKMF-OS/go_kernel/text"
)

// TODO
//go:nosplit
func throw(msg string) {
	text.Printf("throw %s, jump in dead circulate", msg) //TODO
	for {
	}
}

//go:nosplit
func preinit(magic, mbiptr uintptr) {
	text.ClearScreen()
	if ok := simd.AddSSESupprt(); !ok {
		throw("simd add sse support failed")
	}
	gdtInit()
	idtInit()
	mem.KM.FreeRange()
	mem.KM.SetUpKvm()
}
