#include "font.h"

// 看起来原文中的hankaku是可以按位做压缩，因为每一行只有两种状态，而且刚好8个位置
// 因为不想有太多黑盒，所以按照自己的方式生成了hankaku.c文件
static void putfont8(char *vram, int xsize, int x, int y, char c, char *font)
{
	int i, j;
	char *p, d 		/* data */;
	for (i = 0; i < 16; i++) {
		for (j = 0;j < 8;j++) {
			p = vram + (y + i) * xsize + x + j;
			d = font[i * 8 + j];
			if (d == '*') {
				p[0] = c;
			}
		}
	}
}

void putfonts8_asc(char *vram, int xsize, int x, int y, char c, unsigned char *s)
{
	for (; *s != '\0'; s++) {
		putfont8(vram, xsize, x, y, c, hankaku + *s * 16);
		x += 8;
	}
}
