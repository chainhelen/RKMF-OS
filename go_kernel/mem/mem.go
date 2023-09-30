package mem

import (
	"github.com/chainhelen/RKMF-OS/go_kernel/text"
	"unsafe"
)

const (
	MStart      = 100 << 20 // 100M
	MEnd        = 256 << 20 // 256M，注意qemu启动需要指定最大内存空间需要大于这个值
	PageSize    = 4 << 10
	PointerSize = 4 << (^uintptr(0) >> 63)

	// Page table/directory entry flags
	PTE_P  = 0x001 // present
	PTE_W  = 0x002 // writeable
	PTE_U  = 0x004 // User
	PTE_PS = 0x080 //Page Size
)

// refrence kmalloc.c of xv6
//go:notinheap
type run struct {
	next *run
}

type KernelMem struct {
	kpgdir   *pte_t
	start    uintptr
	end      uintptr
	freelist *run
}

// 变量得在全局定义
var KM = KernelMem{
	kpgdir:   nil,
	start:    MStart,
	end:      MEnd,
	freelist: nil,
}

// free the page of physical memory pointed at by v,
// which normally should have been returned by a
// call to kalloc().
//go:nosplit
func (km *KernelMem) free(v uintptr) {
	if v%PageSize != 0 || v < km.start || v > km.end {
		throw("kfree")
	}
	memset(v, 0, PageSize)
	r := (*run)(unsafe.Pointer(v))
	r.next = km.freelist
	km.freelist = r
}

//go:nosplit
func pageRoundUp(sz uintptr) uintptr {
	return (sz + PageSize - 1) &^ (PageSize - 1)
}

//go:nosplit
func pageRoundDown(sz uintptr) uintptr {
	return sz &^ (PageSize - 1)
}

//go:nosplit
func (km *KernelMem) FreeRange() {
	for p := pageRoundUp(km.start); p+PageSize <= km.end; p += PageSize {
		km.free(p)
		// if p != pageRoundUp(km.start) && p != pageRoundUp(km.start+PageSize) {
		// 	text.Printf("addr:0x%x list:0x%x next:0x%x %d\n", p, uintptr(unsafe.Pointer(km.freelist)), uintptr(unsafe.Pointer(km.freelist.next)), uintptr(unsafe.Pointer(km.freelist.next)))
		// }
	}
	text.Printf("free range finish [0x%x, 0x%x), head:0x%x, next:0x%x\n",
		int(km.start), int(km.end), uintptr(unsafe.Pointer(km.freelist)), uintptr(unsafe.Pointer(km.freelist.next)))
}

//go:nosplit
func (km *KernelMem) alloc() uintptr {
	r := km.freelist
	if r == nil {
		throw("km.alloc")
	}
	km.freelist = r.next
	return uintptr(unsafe.Pointer(r))
}

// go:notinheap
type pte_t uintptr

//go:nosplit
func (p *pte_t) Idx(idx int) *pte_t {
	t := pte_t(uintptr(unsafe.Pointer(p)) + uintptr(idx*PointerSize))
	return (*pte_t)(unsafe.Pointer(t))
}

//go:nosplit
func (p *pte_t) Entry() uintptr {
	return uintptr(*p)
}

// https://zhuanlan.zhihu.com/p/327860921
// 1）PML4T(Page Map Level4 Table)及表内的PML4E结构，每个表为4K，内含512个PML4E结构，每个8字节
// 2）PDPT (Page Directory Pointer Table)及表内的PDPTE结构，每个表4K，内含512个PDPTE结构，每个8字节
// 3）PDT (Page Directory Table) 及表内的PDE结构，每个表4K，内含512个PDE结构，每个8字节
// 4）PT(Page Table)及表内额PTE结构，每个表4K，内含512个PTE结构，每个8字节。
// 5）OFFSET：页内偏移量
//go:nosplit
func pageEntryIdx(va uintptr, lv int) int {
	// ((2 << 9)  - 1) 其实就是跟512(2<<9)取模；至于为什么是2<<9，因为一页是4k，而64位寄存器（或者说指针）是8字节，则4k/8 = 512
	return int((va >> (12 + (lv-1)*9)) & ((2 << 9) - 1))
}

//go:nosplit
func walkpgdir(pgdir *pte_t, va uintptr, perm uint64, alloc bool) *pte_t {
	// https://zhuanlan.zhihu.com/p/327860921
	// PML4T、PDPT、PDT、PT、OFFSET
	pg := pgdir
	for lv := 4; lv >= 1; lv-- {
		idx := pageEntryIdx(va, lv)
		pe := pg.Idx(idx)
		if lv == 1 {
			return pe
		}
		if pe.Entry()&PTE_P == 0 {
			if !alloc {
				return nil
			}
			addr := KM.alloc()
			if addr == 0 {
				throw("alloc failed")
			}
			memset(addr, 0, PageSize)
			*pe = pte_t(addr | uintptr(perm))
		}
		pg = (*pte_t)(unsafe.Pointer(*pe))
	}
	return pg
}

//go:nosplit
func mappages(pgdir *pte_t, va, pa uintptr, size uint64, perm uint64) {
	va, last := pageRoundDown(va), pageRoundDown(va+uintptr(size)-1)
	for {
		pte := walkpgdir(pgdir, va, perm, true)
		if pte == nil {
			throw("pte walkpgdir failed")
		}
		text.Printf("*pte 0x%x\n", uintptr(*pte))
		if (*pte)&PTE_P != 0 {
			throw("pte now present")
		}
		*pte = pte_t(uint64(pa) | perm)
		if va == last {
			break
		}
		va += PageSize
		pa += PageSize
	}
}

//go:nosplit
func lcr3(e *pte_t)

//go:nosplit
func (km *KernelMem) SetUpKvm() {
	addr := km.alloc()
	memset(addr, byte(0), PageSize)

	km.kpgdir = (*pte_t)(unsafe.Pointer(addr))
	mappages(km.kpgdir, 0, 0, MEnd, PTE_P|PTE_W|PTE_U)
	lcr3(km.kpgdir)
}

//go:nosplit
func memset(s uintptr, c byte, n int) {
	for i := 0; i < n; i++ {
		pByte := (*byte)(unsafe.Pointer(s + uintptr(i)))
		*pByte = c
	}
}
