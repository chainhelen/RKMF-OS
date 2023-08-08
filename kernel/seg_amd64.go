package kernel

import "unsafe"

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

func gdtInit() {
	// https://wiki.osdev.org/Global_Descriptor_Table  根据这里所说
	// In 64-bit mode, the Base and Limit values are ignored, each descriptor covers the entire
	// linear address space regardless of what they are set to.
	// 可以知道长模式下 base、limit会被忽略
	setGdtDesc(&gdt[SEG_KCODE], Desc_Type_Code_Excute, 0, 0xffffffff, Desc_DPL_0)
	setGdtDesc(&gdt[SEG_KDATA], Desc_Type_Data_Read_Write, 0, 0xffffffff, Desc_DPL_0)
	setGdtDesc(&gdt[SEG_UCODE], Desc_Type_Code_Excute, 0, 0xffffffff, Desc_DPL_3)
	setGdtDesc(&gdt[SEG_UDATA], Desc_Type_Data_Read_Write, 0, 0xffffffff, Desc_DPL_3)

	limit := uint16(unsafe.Sizeof(gdt) - 1)
	base := uint64(uintptr(unsafe.Pointer(&gdt[0])))
	gdtptr[0] = byte(limit)
	gdtptr[1] = byte(limit >> 8)
	gdtptr[2] = byte(base)
	gdtptr[3] = byte(base >> 8)
	gdtptr[4] = byte(base >> 16)
	gdtptr[5] = byte(base >> 24)
	gdtptr[6] = byte(base >> 32)
	gdtptr[7] = byte(base >> 40)
	gdtptr[8] = byte(base >> 48)
	gdtptr[9] = byte(base >> 56)

	lgdt(uintptr(unsafe.Pointer(&gdtptr[0])))
}
