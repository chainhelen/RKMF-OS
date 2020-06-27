#include "windows.h"
#include "timer.h"
#include "keyboard.h"
#include "mouse.h"
#include "idt.h"
#include "sheet.h"
#include "memory.h"
#include "task.h"


void task_b_main(SHEET *sht_back);

void cstart(void) 
{
	init_pit();
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

	// memory
	unsigned int memtotal, memrest;
	struct MEMMAN *memman = (struct MEMMAN *) MEMMAN_ADDR;
	/*内存管理器放在0x00200000,共32KB*/ 
	memtotal = asm_memtest(0x00100000, 0xffffffff); /*检测总内存大小*/ 
	memman_init(memman);/*初始化内存管理器*/ 
	memman_free(memman, 0x00300000, memtotal - 0x00300000);
 	/*内存从3MB处的空间一次性释放可用*/ 
	memrest=memman_total(memman);  /*检测可用内存大小*/ 

	// sheet
	SHTCTL *shtctl;
	SHEET *sht_back, *sht_mouse, *sht_win;
	unsigned char *buf_back, *buf_mouse, *buf_win;
	shtctl = shtctl_init(memman, binfo->vram, binfo->scrnx, binfo->scrny);
	/*新建3个窗口*/
	sht_back  = sheet_alloc(shtctl);
	sht_win   = sheet_alloc(shtctl);
	sht_mouse = sheet_alloc(shtctl);

	buf_back  = (unsigned char *) memman_alloc_4k(memman, binfo->scrnx * binfo->scrny);
	buf_win   = (unsigned char *) memman_alloc_4k(memman, 160 * 68);
	buf_mouse = (unsigned char *) memman_alloc_4k(memman, 16 * 16);

	sheet_setbuf(sht_back, buf_back, binfo->scrnx, binfo->scrny, -1);	//为屏幕图层指定桌面背景的图像信息
	sheet_setbuf(sht_win, buf_win, 160, 68, -1);
	sheet_setbuf(sht_mouse, buf_mouse, 16, 16, 99);

	init_screen(buf_back, binfo->scrnx, binfo->scrny);
	make_window8(buf_win, 160, 68, "windows", 1);
	make_textbox8(sht_win, 8, 28, 144, 34, COL8_FFFFFF);
	init_mouse_cursor8(buf_mouse, 99);

	sheet_slide(sht_back, 0, 0);	//桌面背景显示移到（0,0）处
	sheet_slide(sht_win, binfo->scrnx / 2 - 80, binfo->scrny / 2 - 34);	
	sheet_slide(sht_mouse, binfo->scrnx / 2 - 8, binfo->scrny / 2 - 8);	

	// print info
	putfonts8_asc(buf_back, binfo->scrnx, 30, 60, COL8_FFFFFF, "RKMF OS");
	line(buf_back, binfo->scrnx, 80, COL8_FFFFFF);
	putfonts8_asc(buf_back, binfo->scrnx, 0, 88, COL8_FFFFFF, "Keyboard input : ");

	putfonts8_asc(buf_back, binfo->scrnx, 0, 112, COL8_FFFFFF, "[lcr          ]");
	putfonts8_asc(buf_back, binfo->scrnx, 200, 112, COL8_FFFFFF, "coordinate (    ,     )");

	// show mouse
	int mx, my;
	mx = (binfo->scrnx - 16) / 2;
	my = (binfo->scrny - 28 - 16) / 2;

	// keyboard driver
	char keybuf[32], mousebuf[128];
	fifo8_init(&keyfifo, 32, keybuf);
	fifo8_init(&mousefifo, 128, mousebuf);
	struct MOUSE_DEC mdec;
	init_keyboard();        /*开启鼠标电路*/
	enable_mouse(&mdec);    /*开启鼠标设备*/ 

	// mem
	char s[40];
	mergeMemMsg(s, "TotalMem:    MB   Free:    KB", 
	 	 memtotal / (1024 * 1024), memrest/1024);
 	putfonts8_asc(buf_back, binfo->scrnx, 100, 136, COL8_FFFFFF, s);

	sheet_updown(sht_back,  0);	//桌面背景图层层数为0
	sheet_updown(sht_win,   1);
	sheet_updown(sht_mouse,   2);

	// 定时器
	struct FIFO8 timerfifo;
	char timerbuf[8];
	fifo8_init(&timerfifo, 8, timerbuf);
	TIMER *a_timer = timer_alloc();
	timer_init(a_timer, &timerfifo, 1);
	timer_settime(a_timer, 300);


	TASK *task_a, *task_b;
	// 多任务系统
	task_a = task_init(memman);
	task_b = task_alloc();
	// task_b->tss.esp = memman_alloc_4k(memman, 64 * 1024) + 64 * 1024 - 8;
 	// task_b->tss.eip = (int) &task_b_main;
	// task_b->tss.es = 1 * 8;
	// task_b->tss.cs = 1 * 8;
	// task_b->tss.ss = 1 * 8;
	// task_b->tss.ds = 1 * 8;
	// task_b->tss.fs = 1 * 8;
	// task_b->tss.gs = 1 * 8;
	// *((int *) (task_b->tss.esp + 4)) = (int) sht_back;
	// task_run(task_b);

	 for (;;) 
	 {
	 	 if (fifo8_status(&keyfifo) + fifo8_status(&mousefifo) + fifo8_status(&timerfifo) == 0) {
			task_sleep(task_a);
	 	 	io_stihlt();
	 	 } else {
	 		 if (fifo8_status(&keyfifo) != 0 ) {
	 			 int in = fifo8_get(&keyfifo);
	 			 io_sti();
	 			 char s[40];
	 			 mergeKeyboardMsg(s, "Keyboard input : 0x\0", in);
	 			 int xn = 19;
	 			 boxfill8(buf_back, binfo->scrnx, COL8_008484,  8 * xn, 88, 8 * (xn + 2), 103);
	 			 putfonts8_asc(buf_back, binfo->scrnx, 0, 88, COL8_FFFFFF, s);
				 sheet_refresh(sht_back, 8 * xn, 88, 8 * (xn + 2), 103);
	 		 } else if (fifo8_status(&mousefifo) != 0){
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
	 				 boxfill8(buf_back, binfo->scrnx, COL8_008484, 0, 112, 15 * 8 - 1, 112+15);
	 				 /*数据显示即将更改，把当前位置的颜色填充为背景色*/
	 				 putfonts8_asc(buf_back, binfo->scrnx, 0, 112, COL8_FFFFFF, s);
					 sheet_refresh(sht_back, 0, 112, 15*8-1, 112+15); /* 这里！ */
	 				 /*鼠标即将挪走，把当前位置的颜色填充为背景色*/
					 
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
	 				 boxfill8(buf_back, binfo->scrnx, COL8_008484, 200, 112, 200 + 8 * 24, 112+15);
	 				 /*数据显示即将更改，把当前位置的颜色填充为背景色*/
	 				 putfonts8_asc(buf_back, binfo->scrnx, 200, 112, COL8_FFFFFF, s);
					 sheet_refresh(sht_back, 200, 112, 200 + 8 * 24 + 1, 112+15 + 1);

	 				 /*在新的位置显示出鼠标图形*/
	 				 // line(binfo->vram,binfo->scrnx,132,7);
					 sheet_slide(sht_mouse, mx, my);
	 			 }
	 		 } else if (fifo8_status(&timerfifo) != 0)  {
				 char i = fifo8_get(&timerfifo); /* 首先读入（为了设定起始点） */
				 io_sti();
				 putfonts8_asc(buf_back, binfo->scrnx, 0, 300, COL8_FFFFFF, "3[sec]");
				 sheet_refresh(sht_back, 0, 300, 56, 324);
			 }
	 	 }
	 }
}

void task_b_main(SHEET *sht_back)
{
	struct FIFO8 fifo;
	struct TIMER *timer_put, *timer_1s;
	int i, fifobuf[128], count = 0, count0 = 0;
	char s[12];

	fifo8_init(&fifo, 128, fifobuf);
	timer_put = timer_alloc();
	timer_init(timer_put, &fifo, 1);
	timer_settime(timer_put, 1);
	timer_1s = timer_alloc();
	timer_init(timer_1s, &fifo, 100);
	timer_settime(timer_1s, 100);

	for (;;) {
		count++;
		io_cli();
		if (fifo8_status(&fifo) == 0) {
			io_sti();
		} else {
			i = fifo8_get(&fifo);
			io_sti();
			if (i == 1) {
				// sprintf(s, "%11d", count);
				putfonts8_asc_sht(sht_back, 0, 144, COL8_FFFFFF, COL8_008484, s, 11);
				timer_settime(timer_put, 1);
			} else if (i == 100) {
				// sprintf(s, "%11d", count - count0);
				putfonts8_asc_sht(sht_back, 0, 128, COL8_FFFFFF, COL8_008484, s, 11);
				count0 = count;
				timer_settime(timer_1s, 100);
			}
		}
	}
}
