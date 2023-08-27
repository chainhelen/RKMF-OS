package go_kernel

func preinit() {
	gdtInit()
	idtInit()
}
