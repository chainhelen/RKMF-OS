package go_kernel

func preinit(magic, mbiptr uintptr) {
	gdtInit()
	idtInit()
}
