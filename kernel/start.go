package kernel

func preinit() {
	gdtInit()
	idtInit()
}
