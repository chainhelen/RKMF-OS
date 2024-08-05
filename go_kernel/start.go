package go_kernel

import (
	"github.com/chainhelen/RKMF-OS/drivers/pic_8259"
	"github.com/chainhelen/RKMF-OS/drivers/uart"
	"github.com/chainhelen/RKMF-OS/go_kernel/mem"
	"github.com/chainhelen/RKMF-OS/go_kernel/simd"
	"github.com/chainhelen/RKMF-OS/go_kernel/sys"
	"github.com/chainhelen/RKMF-OS/go_kernel/text"
)

//go:nosplit 用来测试ldflags -E 参数
func Empty() {
}

// TODO
//go:nosplit
func throw(msg string) {
	text.Printf("throw %s, jump in dead circulate", msg) //TODO
	for {
	}
}

//go:nosplit
func preinit(magic, mbiptr uintptr) {
	// text.ClearScreen()
	if ok := simd.SSESupprt(); !ok {
		throw("simd add sse support failed")
	}
	sys.GdtInit()
	sys.IdtInit()
	// text.Printf("dt init finish")

	mem.InitAndSetKvm()
	uart.Init()
	pic_8259.Init()

	// text.Printf("preinit finish")
	for {

	}
}
