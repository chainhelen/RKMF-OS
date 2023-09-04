package go_kernel

import (
	_ "runtime"
)

//go:nosplit 用来测试ldflags -E 参数
func Empty() {
}
