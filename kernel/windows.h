#ifndef WINDOWS_H
#define WINDOWS_H

#define COL8_000000		0
#define COL8_FF0000		1
#define COL8_00FF00		2
#define COL8_FFFF00		3
#define COL8_0000FF		4
#define COL8_FF00FF		5
#define COL8_00FFFF		6
#define COL8_FFFFFF		7
#define COL8_C6C6C6		8
#define COL8_840000		9
#define COL8_008400		10
#define COL8_848400		11
#define COL8_000084		12
#define COL8_840084		13
#define COL8_008484		14
#define COL8_848484		15
#define COL8_3366CC		16
#define COL8_336699		17
#define COL8_008484		18

void boxfill8(unsigned char *vram, int xsize, unsigned char c, int x0, int y0, int x1, int y1);
void init_screen(char *vram, int x, int y);
void init_palette(void);
void line(char *vram,int xsize,int y,char c);

#define BOOTBASE      0x6000
struct BOOTINFO {
	short scrnx, scrny;
	char *vram;
};

#endif
