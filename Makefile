ASM:=nasm
IMG:=a.img
FLOPPY:=/mnt/floppy/

BOOT:=boot.asm
LOADER:=loader.asm
BOOT_BIN:=$(subst .asm,.bin,$(BOOT))
LOADER_BIN:=$(subst .asm,.bin,$(LOADER))
KERNEL_BIN:=kernel.bin

.PHONE: clean

run-bochs: buildimg
	bochs -f bochsrc
run-qemu: buildimg
	qemu -fda $(IMG)

buildimg: $(BOOT_BIN) $(LOADER_BIN) $(KERNEL_BIN)
	bximage -fd -size=1.44 -q $(IMG)
	dd if=$(BOOT_BIN) of=$(IMG) bs=512 count=1 conv=notrunc
	sudo mount -o loop $(IMG) $(FLOPPY)
	sudo cp	$(LOADER_BIN) $(FLOPPY) -v
	sudo cp	$(KERNEL_BIN) $(FLOPPY) -v
	sudo umount $(FLOPPY)

$(BOOT_BIN):
	$(ASM) boot/$(BOOT) -o $(BOOT_BIN)

$(LOADER_BIN):
	$(ASM) loader/$(LOADER) -o $(LOADER_BIN)

$(KERNEL_BIN):
	nasm -f elf kernel/kernel.asm -o kernel/kernel.o 
	gcc -m32 -c -w -fno-builtin kernel/cstart.c -o kernel/cstart.o
	gcc	-m32 -c -w -fno-builtin kernel/windows.c -o kernel/windows.o
	gcc	-m32 -c -w -fno-builtin kernel/idt.c -o kernel/idt.o
	# 话说这不知道链接器ld已经识别_start作为入口了，为什么 kernerl.o还要放在第一位，否则不知道跳到那里去了
	ld -m elf_i386 -s -Ttext 0x30400 kernel/kernel.o  kernel/idt.o kernel/windows.o kernel/cstart.o -o kernel.bin

clean:
	- @rm -rf $(BOOT_BIN)
	- @rm -rf loader.bin
	- @rm -rf kernel/*.o
	- @rm -rf kernel.bin
	- @rm -rf $(IMG)
	- @rm -rf *.swp
