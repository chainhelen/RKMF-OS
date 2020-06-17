gcc -m32 -c -fno-builtin -c b.c -o b.o
nasm -f elf -o a.o a.asm                      
ld -m elf_i386 -s -Ttext 0x30400 -o c  a.o b.o 
