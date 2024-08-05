package pic_8259

// https://wiki.osdev.org/8259_PIC

import (
	"github.com/chainhelen/RKMF-OS/go_kernel/Interrupts"
	"github.com/chainhelen/RKMF-OS/go_kernel/sys"
)

/* reinitialize the PIC controllers, giving them specified vector offsets
   rather than 8h and 70h, as configured by default */

// #define ICW1_ICW4	0x01		/* Indicates that ICW4 will be present */
// #define ICW1_SINGLE	0x02		/* Single (cascade) mode */
// #define ICW1_INTERVAL4	0x04		/* Call address interval 4 (8) */
// #define ICW1_LEVEL	0x08		/* Level triggered (edge) mode */
// #define ICW1_INIT	0x10		/* Initialization - required! */
//
// #define ICW4_8086	0x01		/* 8086/88 (MCS-80/85) mode */
// #define ICW4_AUTO	0x02		/* Auto (normal) EOI */
// #define ICW4_BUF_SLAVE	0x08		/* Buffered mode/slave */
// #define ICW4_BUF_MASTER	0x0C		/* Buffered mode/master */
// #define ICW4_SFNM	0x10		/* Special fully nested (not) */
// }

const (
	PIC1         uint16 = 0x20 /* IO base address for master PIC */
	PIC2         uint16 = 0xA0 /* IO base address for slave PIC */
	PIC1_COMMAND uint16 = PIC1
	PIC1_DATA    uint16 = (PIC1 + 1)
	PIC2_COMMAND uint16 = PIC2
	PIC2_DATA    uint16 = (PIC2 + 1)

	ICW1_INIT byte = 0x10 /* Initialization - required! */
	ICW1_ICW4 byte = 0x01 /* Indicates that ICW4 will be present */

	ICW4_8086 byte = 0x01 /* 8086/88 (MCS-80/85) mode */
)

/*
	IRQ	Description
	0	Programmable Interrupt Timer Interrupt
	1	Keyboard Interrupt
	2	Cascade (used internally by the two PICs. never raised)
	3	COM2 (if enabled)
	4	COM1 (if enabled)
	5	LPT2 (if enabled)
	6	Floppy Disk
	7	LPT1 / Unreliable "spurious" interrupt (usually)
	8	CMOS real-time clock (if enabled)
	9	Free for peripherals / legacy SCSI / NIC
	10	Free for peripherals / SCSI / NIC
	11	Free for peripherals / SCSI / NIC
	12	PS2 Mouse
	13	FPU / Coprocessor / Inter-processor
	14	Primary ATA Hard Disk
	15	Secondary ATA Hard Disk
*/

const (
	IRQ_TIMER    byte = 0
	IRQ_KEYBOARD byte = 1
	IRQ_CASCADE  byte = 2
	IRQ_COM2     byte = 3
	IRQ_COM1     byte = 4
	IRQ_LPT2     byte = 5
	IRQ_FLOPPY   byte = 6
	IRQ_LPT1     byte = 7
	IRQ_RTC      byte = 8
	IRQ_CMOS     byte = 9
	IRQ_MOUSE    byte = 0xC
)

//go:nosplit
func Init() {
	// 将pic的IRQ0位置设置为0x20中断向量号
	remap(interrupts.VectorAssignment, interrupts.VectorAssignment+8)
	EnableIRQ(IRQ_CASCADE)
}

// https://wiki.osdev.org/Inline_Assembly/Examples#IO_WAIT
//go:nosplit
func io_wait() {
	sys.Outb(0x80, 0)
}

// https://wiki.osdev.org/8259_PIC#Initialisation 初始化代码如下，我们翻译成golang版本
//go:nosplit
func remap(offset1 byte, offset2 byte) {
	// ICM1
	sys.Outb(PIC1_COMMAND, ICW1_INIT|ICW1_ICW4)
	io_wait()
	sys.Outb(PIC2_COMMAND, ICW1_INIT|ICW1_ICW4)
	io_wait()

	// ICM2
	sys.Outb(PIC1_DATA, offset1)
	io_wait()
	sys.Outb(PIC2_DATA, offset2)
	io_wait()

	// ICM3
	sys.Outb(PIC1_DATA, 4)
	io_wait()
	sys.Outb(PIC2_DATA, 2)
	io_wait()

	// ICM4
	sys.Outb(PIC1_DATA, ICW4_8086)
	io_wait()
	sys.Outb(PIC2_DATA, ICW4_8086)
	io_wait()
}

//go:nosplit
func EnableIRQ(irqNumber byte) {
	// 注意 0 代表unmask，即开启；1代表mask关闭
	// OCW1 用来控制IRQ的打开和关闭; https://www.eeeguide.com/8259-programmable-interrupt-controller/#google_vignette
	if irqNumber >= IRQ_RTC { // 说明在从板上面
		irqNumber -= IRQ_RTC
		sys.Outb(PIC2_DATA, byte(sys.Inb(PIC2_DATA)&^(1<<irqNumber)))
		return
	}
	sys.Outb(PIC1_DATA, byte(sys.Inb(PIC1_DATA)&^(1<<irqNumber)))
}
