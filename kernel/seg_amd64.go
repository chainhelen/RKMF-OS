package kernel

import (
	"unsafe"
)

// xv6 https://github.com/mit-pdos/xv6-public/blob/master/mmu.h#L15
const (
	SEG_KCODE = 1 // kernel code
	SEG_KDATA = 2 // kernel data+stack
	SEG_UCODE = 3 // user code
	SEG_UDATA = 4 // user data+stack
	SEG_TSS   = 5 // this process's task state
)

type gdtSegDesc [8]byte

var (
	gdt    [7]gdtSegDesc
	gdtptr [10]byte
	tss    [26]uint32
)

// 段描述符，这里有介绍 https://github.com/MintCN/linux-insides-zh/blob/master/Booting/linux-bootstrap-2.md#%E4%BF%9D%E6%8A%A4%E6%A8%A1%E5%BC%8F
func setGdtDesc(desc *gdtSegDesc, _type, _base, _limit, _dpl uint32) {
	// limit 0-15
	desc[0] = byte(_limit)
	desc[1] = byte(_limit >> 8)

	// base 0-15
	desc[2] = byte(_base)
	desc[3] = byte(_base >> 8)

	// base 16~23
	desc[4] = byte(_base >> 16)

	// 0x90 将P、S 两个位设置为 1（即 45、47两位 设置为 1）
	// 设置_dpl（_dpl << 5左移5是为了让_dpl数值在正确的数值位置上）
	desc[5] = byte((0x90) | (_dpl << 5) | (_type))

	// limit 16-19，这里要与0x07与一下，主要要把 _limit>>16的数字共8位的前4位置零，这样只保留后4位，即limit的16-19
	// 如果是64bit 代码段，那么Long 需要置1 且 DB 需要置零
	if _type&Desc_Type_Code_Excute != 0 {
		desc[6] = 0x20 | (byte(_limit>>16) & 0x7)
	} else {
		// 0x80 是置D/B，32位中数据段、栈段代表是32位，目前在64位干什么的不太清楚，看描述64bit代码段需要置空；看xv6中也是置为1了 TODO 去掉看看
		desc[6] = 0x80 | (byte(_limit>>16) & 0x7)
	}
	desc[7] = byte(_base >> 24)
}

func setTssDesc(lo, hi *gdtSegDesc, addr, limit uintptr) {
	// tss limit 0-15
	lo[0] = byte(limit)
	lo[1] = byte(limit >> 8)
	// tss base 0-15
	lo[2] = byte(addr)
	lo[3] = byte(addr >> 8)
	// tss base 16-23
	lo[4] = byte(addr >> 16)
	// type 64 bit tss, P = 1
	lo[5] = 0x89
	// limit 16-19 and AVL = 1
	lo[6] = 0x80 | byte(limit>>16)&0x07
	lo[7] = byte(addr >> 24)

	for i := 8; i < 16; i++ {
		hi[i] = byte(addr >> (32 + 8*(i-8)))
	}
}

const (
	Desc_DPL_0 = 0
	Desc_DPL_3 = 3

	Desc_Type_Data_Read_Accessed   = 0x1
	Desc_Type_Data_Read_Write      = 0x2
	Desc_Type_Data_Read_ExpandDown = 0x4
	Desc_Type_Code_Excute          = 0x8
	Desc_Type_Code_Accessed        = 0x1
	Desc_Type_Code_Read            = 0x2
	Desc_Type_Code_Conforming      = 0x4

	Desc_Flags_L = 0x2
)

func lgdt(gdtptr uintptr)
func ltr(sel uintptr) // xv6是32bit，对应ltr参数是16位；这里64bit下的ltr的参数是32位。
func reloadCS()

func gdtInit() {
	// https://wiki.osdev.org/Global_Descriptor_Table  根据这里所说
	// In 64-bit mode, the Base and Limit values are ignored, each descriptor covers the entire
	// linear address space regardless of what they are set to.
	// 可以知道长模式下 base、limit会被忽略
	setGdtDesc(&gdt[SEG_KCODE], Desc_Type_Code_Excute, 0, 0xffffffff, Desc_DPL_0)
	setGdtDesc(&gdt[SEG_KDATA], Desc_Type_Data_Read_Write, 0, 0xffffffff, Desc_DPL_0)
	setGdtDesc(&gdt[SEG_UCODE], Desc_Type_Code_Excute, 0, 0xffffffff, Desc_DPL_3)
	setGdtDesc(&gdt[SEG_UDATA], Desc_Type_Data_Read_Write, 0, 0xffffffff, Desc_DPL_3)
	setTssDesc(&gdt[SEG_TSS], &gdt[SEG_TSS+1], uintptr(unsafe.Pointer(&tss[0])), uintptr(unsafe.Sizeof(tss))-1)

	limit := uint16(unsafe.Sizeof(gdt) - 1)
	base := uint64(uintptr(unsafe.Pointer(&gdt[0])))
	gdtptr[0] = byte(limit)
	gdtptr[1] = byte(limit >> 8)
	for i := 2; i < 10; i++ {
		gdtptr[i] = byte(base >> (8 * (i - 2)))
	}

	lgdt(uintptr(unsafe.Pointer(&gdtptr[0])))
	// https://wiki.osdev.org/Task_State_Segment
	// The only interesting fields are SS0 and ESP0. Whenever a system call occurs, the CPU gets the SS0 and ESP0-value
	// in its TSS and assigns the stack-pointer to it.
	ltr(SEG_TSS << 3)

	// https://stackoverflow.com/questions/34264752/change-gdt-and-update-cs-while-in-long-mode
	// reloadCS() 这里应该不需要reload cs
}

var idt [256]idtSetDesc

type idtSetDesc struct {
	// copy from https://wiki.osdev.org/Interrupt_Descriptor_Table
	//	uint16_t offset_1;        // offset bits 0..15
	//	uint16_t selector;        // a code segment selector in GDT or LDT
	//	uint8_t  ist;             // bits 0..2 holds Interrupt Stack Table offset, rest of bits zero.
	//	uint8_t  type_attributes; // gate type, dpl, and p fields
	//	uint16_t offset_2;        // offset bits 16..31
	//	uint32_t offset_3;        // offset bits 32..63
	//	uint32_t zero;            // reserved

	Offset1        uint16
	Selector       uint16
	Ist            uint8
	TypeAttributes uint8
	Offset2        uint16
	Offset3        uint32
	Zero           uint32
}

//go:nosplit
func setIdtDesc(desc *idtSetDesc, selector uint16, addr uintptr, dpl byte) {
	desc.Offset1 = uint16(addr & 0xffff)
	desc.Selector = selector << 3
	desc.Ist = 0
	desc.TypeAttributes = 0x8e | (dpl << 4)
	desc.Offset2 = uint16(addr >> 16 & 0xffff)
	desc.Offset3 = uint32(addr>>32) & 0xffffffff
}

// go::linkname FuncPC runtime.funcPC
func FuncPC(interface{}) uintptr

const (
	T_SYSCALL = 64 // system call
)

var (
	idtptr [10]byte
)

//go:nosplit
func lidt(idtptr uintptr)

//go:generate go run ./vector_gen/main.go
func idtInit() {
	// https://wiki.osdev.org/Interrupt_Descriptor_Table
	for i := 0; i < 256; i++ {
		setIdtDesc(&idt[i], SEG_KCODE, FuncPC(vectors[i]), Desc_DPL_0)
	}
	setIdtDesc(&idt[T_SYSCALL], SEG_KCODE, FuncPC(vectors[T_SYSCALL]), Desc_DPL_3)

	limit := uint16(unsafe.Sizeof(idt) - 1)
	base := uint64(uintptr(unsafe.Pointer(&idt[0])))

	idtptr[0] = byte(limit)
	idtptr[1] = byte(limit >> 8)
	for i := 2; i < 10; i++ {
		idtptr[i] = byte(base >> (8 * (i - 2)))
	}

	lidt(uintptr(unsafe.Pointer(&idtptr)))
}
