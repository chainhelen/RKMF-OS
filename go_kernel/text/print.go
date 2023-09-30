package text

import (
	_ "unsafe"
)

const (
	videoBuf = uintptr(0xb8000)
	maxW     = 160
	maxH     = 25
)

var (
	videoOff = uintptr(0)
	digits   = []byte("0123456789ABCDEF")
)

//go:linkname memset github.com/chainhelen/RKMF-OS/go_kernel/mem.memset
func memset(s uintptr, c byte, n int)

//go:linkname throw github.com/chainhelen/RKMF-OS/go_kernel.throw
func throw(msg string)

//go:nosplit
func ClearScreen() {
	videoOff = 0
	for i := 0; i < maxW*maxH; i += 2 {
		putc(' ')
	}
	videoOff = 0
}

//go:nosplit
func clearOneLine(w int) {
	off := videoOff

	videoOff = maxW * uintptr(w)
	for i := 0; i < maxW; i += 2 {
	}

	videoOff = off
}

//go:nosplit
func o_print_str(s string) {
	for i := 0; i < len(s); i++ {
		putc(s[i])
	}
}

//go:nosplit
func putc(c byte) {
	switch c {
	case '\n':
		videoOff = (videoOff/maxW + 1) * maxW
		if videoOff >= maxH*maxW { // 超过了就重头覆盖回来
			videoOff = 0
		}
		t := videoOff
		for i := 0; i < maxW; i += 2 {
			memset(videoBuf+t, ' ', 1)
			t++
			memset(videoBuf+t, 0x7, 1)
			t++
		}
	default:
		memset(videoBuf+videoOff, c, 1)
		videoOff++
		//红底绿字，属性字节为：01000010B = 0x42；
		//红底闪烁绿字，属性字节为：11000010B = 0xc2；
		//红底高亮绿字，属性字节为：01001010B = 0x4a；
		//黑底白字，属性字节为：00000111B = 0x07；
		//白底蓝字，属性字节为：01110001B = 0x71。
		memset(videoBuf+videoOff, 0x7, 1)
		videoOff++
		if videoOff >= maxH*maxW { // 超过了就重头覆盖回来
			videoOff = 0
		}
	}
}

//go:nosplit
func putint(a interface{}, base uint64, sgn bool) {
	xx := int64(0)

	switch a.(type) {
	case int:
		xx = int64(a.(int))
	case uint64:
		xx = int64(a.(uint64))
	case int64:
		xx = a.(int64)
	case uintptr:
		xx = int64(a.(uintptr))
	default:
		putc('!')
		for {
		}
		//		throw("panic, unknown type")
	}

	buf := [22]byte{}
	x := uint64(0)

	neg := false
	if sgn && xx < 0 {
		neg = true
		x = uint64(-1 * xx)
	} else {
		x = uint64(xx)
	}
	i := 0
	for {
		buf[i] = digits[x%base]
		if x = x / base; x == 0 {
			break
		}
		i++
	}
	if neg {
		buf[i] = '-'
		i++
	}
	for i >= 0 {
		putc(buf[i])
		i--
	}
}

// https://github.com/golang/go/wiki/minimumrequirements#amd64
// => 0x000000000325771a <+26>:	mov    %rax,(%rsp)
//   0x000000000325771e <+30>:	movq   $0x3,0x8(%rsp)
//   0x0000000003257727 <+39>:	xorps  %xmm0,%xmm0
//   0x000000000325772a <+42>:	movups %xmm0,0x10(%rsp）
// 需要关闭see优化，否则在xorps %xmm0,%xmm0 会失败
//go:nosplit
func Printf(format string, a ...interface{}) {
	// copy from  Printf.c in xv6
	// support %d %s %x，不支持汉字
	var (
		state = byte(0)
		ap    = 0
		s     = ""
	)

	for i := 0; i < len(format); i++ {
		c := format[i] & 0xff
		if state == 0 {
			if c == '%' {
				state = '%'
			} else {
				putc(c)
			}
		} else if state == '%' {
			if c == 'd' {
				putint(a[ap], 10, true)
				ap++
			} else if c == 'x' {
				putint(a[ap], 16, false)
				ap++
			} else if c == 's' {
				s = a[ap].(string)
				o_print_str(s)
				ap++
			} else if c == 'c' {
				putc(c)
				ap++
			} else if c == '%' {
				putc(c)
			} else {
				putc('%')
				putc(c)
			}
			state = 0
		}
	}
}
