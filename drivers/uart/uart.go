package uart

import "github.com/chainhelen/RKMF-OS/go_kernel/sys"

// Intel 8250 serial port (UART).

/*
https://en.wikibooks.org/wiki/Serial_Programming/8250_UART_Programming

Base Address	DLAB	I/O Access	Abbrv.	Register Name
+0	0	Write	THR	Transmitter Holding Buffer
+0	0	Read	RBR	Receiver Buffer
+0	1	Read/Write	DLL	Divisor Latch Low Byte
+1	0	Read/Write	IER	Interrupt Enable Register
+1	1	Read/Write	DLH	Divisor Latch High Byte
+2	x	Read	IIR	Interrupt Identification Register
+2	x	Write	FCR	FIFO Control Register
+3	x	Read/Write	LCR	Line Control Register
+4	x	Read/Write	MCR	Modem Control Register
+5	x	Read	LSR	Line Status Register
+6	x	Read	MSR	Modem Status Register
+7	x	Read/Write	SR	Scratch Register
*/
const (
	COM1_Base uint16 = 0x3f8
	COM2_Base uint16 = 0x2f8

	IER_Offset uint16 = 1
	FCR_Offset uint16 = 2
	LCR_Offset uint16 = 3
	MCR_Offset uint16 = 4

	Latch_Low  uint16 = 0x00
	Latch_High uint16 = 0x01
)

//go:nosplit
func Init() {
	// https://en.wikibooks.org/wiki/Serial_Programming/8250_UART_Programming#FIFO_Control_Register
	// Writing a "0" to bit 0 will disable the FIFOs, in essence turning the UART into 8250 compatibility mode.
	sys.Outb(COM1_Base+FCR_Offset, 0)

	// uart的初始化可以查看官方文档里的面set波特率
	// https://en.wikibooks.org/wiki/Serial_Programming/8250_UART_Programming#Divisor_Latch_Bytes
	/*
		const
		  COM1_Base = $3F8;
		  COM2_Base = $2F8;
		  LCR_Offset = $03;
		  Latch_Low = $00;
		  Latch_High = $01;

		procedure SetBaudRate(NewRate: Word);
		var
		  DivisorLatch: Word;
		begin
		  DivisorLatch := 115200 div NewRate;
		  Port[COM1_Base + LCR_Offset] := Port[COM1_Base + LCR_Offset] or $80; {Set DLAB}
		  Port[COM1_Base + Latch_High] := DivisorLatch shr 8;
		  Port[COM1_Base + Latch_Low] := DivisorLatch and $FF;
		  Port[COM1_Base + LCR_Offset] := Port[COM1_Base + LCR_Offset] and $7F; {Clear DLAB}
		end;
	*/
	NewRate := 9600
	lcrValue := sys.Inb(COM1_Base + LCR_Offset)
	DivisorLatch := 115200 / NewRate
	sys.Outb(COM1_Base+LCR_Offset, lcrValue|0x80)
	sys.Outb(COM1_Base+Latch_High, byte(DivisorLatch>>8))
	sys.Outb(COM1_Base+Latch_Low, byte(DivisorLatch&0xFF))
	sys.Outb(COM1_Base+LCR_Offset, lcrValue&0x7F)

	sys.Outb(COM1_Base+MCR_Offset, 0x00)
	/*
		Interrupt Enable Register (IER)
		Bit	Notes
		7	Reserved
		6	Reserved
		5	Enables Low Power Mode (16750)
		4	Enables Sleep Mode (16750)
		3	Enable Modem Status Interrupt
		2	Enable Receiver Line Status Interrupt
		1	Enable Transmitter Holding Register Empty Interrupt
		0	Enable Received Data Available Interrupt
	*/
	sys.Outb(COM1_Base+IER_Offset, 0x01) // enable received interrupt

}
