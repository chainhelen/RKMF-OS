ASM=nasm

.PHONE: clean

run-bochs: buildimg
	bochs -f bochsrc
run-qemu: buildimg
	qemu -fda a.img

buildimg:	boot.bin
	dd if=boot.bin	of=a.img bs=20480 count=1 conv=notrunc

boot.bin: boot.asm
	$(ASM)	$(ASMBFLAGS) -o $@ $<

clean:
	rm boot.bin
	rm a.img

