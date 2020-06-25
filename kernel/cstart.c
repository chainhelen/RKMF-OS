#include "windows.h"
#include "keyboard.h"
#include "mouse.h"
#include "idt.h"

void cstart(void) 
{
	// 一定要先load idt寄存器然后在desktop
	// 因为desktop里面有个死循环，否则中断向量表永远不生效
	init_idt();
	load_idt_reg();

	desktop();
}

extern struct FIFO8 keyfifo;
extern struct FIFO8 mousefifo;

void desktop(void)
{
	struct BOOTINFO *binfo = (struct BOOTINFO *) BOOTBASE;
	init_palette();
	init_screen(binfo->vram, binfo->scrnx, binfo->scrny);

	// print info
	putfonts8_asc(binfo->vram, binfo->scrnx, 30, 60, COL8_FFFFFF, "RKMF OS");
	line(binfo->vram, binfo->scrnx, 80, COL8_FFFFFF);
	putfonts8_asc(binfo->vram, binfo->scrnx, 0, 88, COL8_FFFFFF, "Keyboard input : ");
	line(binfo->vram,binfo->scrnx,108,7);

	putfonts8_asc(binfo->vram, binfo->scrnx, 0, 112, COL8_FFFFFF, "[lcr          ]");
	putfonts8_asc(binfo->vram, binfo->scrnx, 200, 112, COL8_FFFFFF, "coordinate (    ,     )");
	line(binfo->vram,binfo->scrnx,132,7);

	// show mouse
	char mcursor[256];
	int mx, my;
	mx = (binfo->scrnx - 16) / 2;
	my = (binfo->scrny - 28 - 16) / 2;
	init_mouse_cursor8(mcursor, COL8_008484);
	putblock8_8(binfo->vram, binfo->scrnx, 16, 16, mx, my, mcursor, 16);

	// keyboard driver
	char keybuf[32], mousebuf[128];
	fifo8_init(&keyfifo, 32, keybuf);
	fifo8_init(&mousefifo, 128, mousebuf);
	struct MOUSE_DEC mdec;
	init_keyboard();        /*开启鼠标电路*/
	enable_mouse(&mdec);    /*开启鼠标设备*/ 
	//
	for (;;) 
	{
		 if (fifo8_status(&keyfifo) + fifo8_status(&mousefifo) == 0) {
		 	io_stihlt();
		 } else {
			 if (fifo8_status(&keyfifo) != 0 ) {
				 int in = fifo8_get(&keyfifo);
				 io_sti();
				 char s[40];
				 mergeKeyboardMsg(s, "Keyboard input : 0x\0", in);
				 int xn = 19;
				 boxfill8(binfo->vram, binfo->scrnx, COL8_008484,  8 * xn, 88, 8 * (xn + 2), 103);
				 putfonts8_asc(binfo->vram, binfo->scrnx, 0, 88, COL8_FFFFFF, s);
				 line(binfo->vram,binfo->scrnx,108,7);
			 } else {
				 int in = fifo8_get(&mousefifo);
				 io_sti();
				 if (mouse_decode(&mdec, in) != 0) {
					 char s[40];
					 mergeMouseLcrMsg(s, "[lcr 0000 0000]\0", mdec.x, mdec.y);
					 if ((mdec.btn & 0x01) != 0) {
						 s[1] = 'L';
					 }
					 if ((mdec.btn & 0x02) != 0) {
						 s[3] = 'R';
					 }
					 if ((mdec.btn & 0x04) != 0) {
						 s[2] = 'C';
					 }
					 boxfill8(binfo->vram, binfo->scrnx, COL8_008484, 0, 112, 0 + 15 * 8 - 1, 112+15);
					 /*数据显示即将更改，把当前位置的颜色填充为背景色*/
					 putfonts8_asc(binfo->vram, binfo->scrnx, 0, 112, COL8_FFFFFF, s);
					 boxfill8(binfo->vram, binfo->scrnx, COL8_008484, mx, my, mx + 15, my + 15);
					 /*鼠标即将挪走，把当前位置的颜色填充为背景色*/
					 line(binfo->vram,binfo->scrnx,132,7);
					 mx += mdec.x;
					 my += mdec.y;
					 if (mx < 0) {
						 mx = 0;
					 }
					 if (my < 0) {
						 my = 0;
					 }
					 if (mx > binfo->scrnx - 16) {
						 mx = binfo->scrnx - 16;
					 }
					 if (my > binfo->scrny - 16) {
						 my = binfo->scrny - 16;
					 }
					 mergeMouseCoordinateMsg(s, "coordinate (0000, 0000)", mx, my);
					 boxfill8(binfo->vram, binfo->scrnx, COL8_008484, 200, 112, 200 + 8 * 24, 112+15);
					 /*数据显示即将更改，把当前位置的颜色填充为背景色*/
					 putfonts8_asc(binfo->vram, binfo->scrnx, 200, 112, COL8_FFFFFF, s);
					 putblock8_8(binfo->vram, binfo->scrnx, 16, 16, mx, my, mcursor, 16);
					 /*在新的位置显示出鼠标图形*/
					 line(binfo->vram,binfo->scrnx,132,7);
				 }
			 }
		 }
	}
}
