package mem

import (
	_ "unsafe"
)

//go:linkname throw github.com/chainhelen/RKMF-OS/go_kernel.throw
func throw(msg string)
