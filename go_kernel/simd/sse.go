package simd

//go:nosplit
func sseInit()

//go:nosplit
func checkSSE() (ok bool)

//go:nosplit
func SSESupprt() bool {
	sseInit()
	return checkSSE()
}
