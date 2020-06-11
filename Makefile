ASM=nasm

.PHONE: clean

run-bochs: buildimg
	bochs -f bochsrc
run-qemu: buildimg
	qemu -fda a.img

buildimg:	boot.bin loader.bin
	dd if=boot.bin	of=a.img bs=512 count=1 conv=notrunc
	dd if=loader.bin of=a.img bs=512 count=4 conv=notrunc seek=1

boot.bin: boot.asm
	$(ASM)	$(ASMBFLAGS) -o $@ $<

loader.bin: loader.asm
	$(ASM)	$(ASMBFLAGS) -o $@ $<

clean:
	- @rm -rf boot.bin
	- @rm -rf loader.bin
	- @rm -rf a.img
