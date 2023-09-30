package simd

//go:nosplit
func sseInit()

//go:nosplit
func checkSSE() (ok bool)

//go:nosplit
func AddSSESupprt() bool {
	sseInit()
	return checkSSE()
}
