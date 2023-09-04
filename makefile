CC = gcc
LD = ld
GO = go
CCFLAGS = -fno-pic -static -fno-builtin -fno-strict-aliasing -g -O0 -Wall -Werror -fno-omit-frame-pointer -I. -nostdinc -fno-pie -m64
/*LDFLAGS = -N -e _start -Ttext 0x3200000 -m elf_x86_64*/
LDFLAGS = -T ./boot/kernel64.ld -m elf_x86_64

go.elf:
	go build -ldflags="-E github.com/chainhelen/RKMF-OS/go_kernel.rt0 -T 0x3200000" -gcflags="all=-N -l" -o go.elf ./app
bootloader.elf: boot.o main.o
	$(LD) $(LDFLAGS) -o $@ $^
boot.o: ./boot/boot.S
	$(CC) $(CCFLAGS) -o $@ -c $^
main.o: ./boot/main.c
	$(CC) $(CCFLAGS) -o $@ -c $^
qemu: bootloader.elf go.elf
	qemu-system-x86_64 -no-reboot -kernel bootloader.elf -initrd go.elf
qemu-gdb: bootloader.elf go.elf
	qemu-system-x86_64 -s -S -no-reboot -kernel bootloader.elf -initrd go.elf
clean:
	rm -rf *.o && rm bootloader.elf && rm go.elf
