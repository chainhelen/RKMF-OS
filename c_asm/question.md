ld -m elf_i386 -s -Ttext 0x30400 -o c  a.o b.o 

注意这个 a.o b.o 的顺序不能反，可以对比正反的结果

正常的编译
```
# ld -m elf_i386 -s -Ttext 0x30400 -o c  a.o b.o 
# objdump -D c

00030400 <.text>:
   30400:	66 8c c8             	mov    %cs,%ax
   30403:	e8 3c 00 00 00       	call   0x30444
   30408:	c6 05 00 8f 0b 00 43 	movb   $0x43,0xb8f00
   3040f:	c6 05 01 8f 0b 00 0c 	movb   $0xc,0xb8f01
   30416:	c3                   	ret    

可以看到这个入口确实我们定义的_start标签
```

一旦反过来
```
# ld -m elf_i386 -s -Ttext 0x30400 -o c  b.o a.o 
# objdump -D c

00030400 <.text>:
   30400:	55                   	push   %ebp
   30401:	89 e5                	mov    %esp,%ebp
   30403:	53                   	push   %ebx
   30404:	83 ec 04             	sub    $0x4,%esp
   30407:	e8 1c 00 00 00       	call   0x30428
   3040c:	81 c3 f4 1b 00 00    	add    $0x1bf4,%ebx

可以看到这个入口是我们b.asm的.text代码
```
