package sys

//go:nosplit
func Outb(port uint16, data byte)

//go:nosplit
func Inb(port uint16) byte
