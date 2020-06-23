#include "windows.h"

void boxfill8(unsigned char *vram, int xsize, unsigned char c, int x0, int y0, int x1, int y1)
{
	int x, y;
	for (y = y0; y <= y1; y++) {
		for (x = x0; x <= x1; x++)
			vram[y * xsize + x] = c;
	}
}

void init_screen(char *vram, int x, int y)
{
	boxfill8(vram, x, COL8_008484,  0,     0,      x -  1, y - 29);
	boxfill8(vram, x, COL8_C6C6C6,  0,     y - 28, x -  1, y - 28);
	boxfill8(vram, x, COL8_FFFFFF,  0,     y - 27, x -  1, y - 27);
	boxfill8(vram, x, COL8_C6C6C6,  0,     y - 26, x -  1, y -  1);

	boxfill8(vram, x, COL8_FFFFFF,  3,     y - 24, 59,     y - 24);
	boxfill8(vram, x, COL8_FFFFFF,  2,     y - 24,  2,     y -  4);
	boxfill8(vram, x, COL8_848484,  3,     y -  4, 59,     y -  4);
	boxfill8(vram, x, COL8_848484, 59,     y - 23, 59,     y -  5);
	boxfill8(vram, x, COL8_000000,  2,     y -  3, 59,     y -  3);
	boxfill8(vram, x, COL8_000000, 60,     y - 24, 60,     y -  3);

	boxfill8(vram, x, COL8_848484, x - 47, y - 24, x -  4, y - 24);
	boxfill8(vram, x, COL8_848484, x - 47, y - 23, x - 47, y -  4);
	boxfill8(vram, x, COL8_FFFFFF, x - 47, y -  3, x -  4, y -  3);
	boxfill8(vram, x, COL8_FFFFFF, x -  3, y - 24, x -  3, y -  3);

}

static void set_palette(int start, int end, unsigned char *rgb)
{
	int i;
	out_byte(0x03c8, start);
	for (i = start; i <= end; i++) {
		out_byte(0x03c9, rgb[0] / 4);
		out_byte(0x03c9, rgb[1] / 4);
		out_byte(0x03c9, rgb[2] / 4);
		rgb += 3;
	}
}



void init_palette(void)
{
	static unsigned char table_rgb[19 * 3] = {
		0x00, 0x00, 0x00,	/*  0 */
		0xff, 0x00, 0x00,	/*  1 */
		0x00, 0xff, 0x00,	/*  2 */
		0xff, 0xff, 0x00,	/*  3 */
		0x00, 0x00, 0xff,	/*  4 */
		0xff, 0x00, 0xff,	/*  5 */
		0x00, 0xff, 0xff,	/*  6 */
		0xff, 0xff, 0xff,	/*  7 */
		0xc6, 0xc6, 0xc6,	/*  8 */
		0x84, 0x00, 0x00,	/*  9 */
		0x00, 0x84, 0x00,	/* 10 */
		0x84, 0x84, 0x00,	/* 11 */
		0x00, 0x00, 0x84,	/* 12 */
		0x84, 0x00, 0x84,	/* 13 */
		0x00, 0x84, 0x84,   /* 14 */
		0x84, 0x84, 0x84,	/* 15 */
		0x33, 0x66, 0xcc,	/* 16 */
		0x33, 0x66, 0x99,	/* 17 */
        0x00, 0x84, 0x84	/* 18 */
	};
	set_palette(0, 18, table_rgb);
}


void line(char *vram,int xsize,int y,char c)
{
	int i=0;
	for (i = 0; i <=xsize-1; i++) {
		*(vram + y*xsize + i)=c;
	}
}


