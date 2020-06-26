#include "memory.h"

#ifndef SHEET_H
#define SHEET_H

#define MAX_SHEETS 256

typedef struct _SHEET{
	unsigned char *buf;
	int width, height, bxsize, bysize, vx0, vy0, col_inv, weight, flags;
	struct SHTCTL *ctl;
	struct TASK *task;
} SHEET;

typedef struct SHTCTL{
	unsigned char * vram, *map; //vram对应显存地址
	int xsize, ysize, top; //xsize, ysize 代表整个显示界面的宽和高，top 表示当前要显示几个图层
	SHEET *sheets[MAX_SHEETS]; //sheets0 是用来存储图层对象的数组
	SHEET sheets0[MAX_SHEETS]; //sheets指向下面图层数组中的对应图层对象
}SHTCTL;

void sheet_setWeight(SHTCTL *ctl, SHEET *sht, int weight);
SHTCTL *shtctl_init(MEMMAN *memman, unsigned char *vram, int xsize, int ysize);
SHEET *sheet_alloc(SHTCTL *ctl);
void sheet_setbuf(SHEET *sht, unsigned char *buf, int xsize, int ysize, int col_inv);

#endif
