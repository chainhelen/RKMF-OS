#include "fifo.h"
#include "windows.h"
#include "mouse.h"

struct 	FIFO8 	mousefifo;

void enable_mouse(struct MOUSE_DEC *mdec)
{
	wait_KBC_sendready();
	out_byte(PORT_KEYCMD, KEYCMD_SENDTO_MOUSE);
	wait_KBC_sendready();
	out_byte(PORT_KEYDAT, MOUSECMD_ENABLE);

	mdec->phase = 0;
	return;
}

void mousehandler() 
{
	unsigned char data;
	out_byte(PIC1_OCW2, 0x64);	/* IRQ-12PIC1 */
	out_byte(PIC0_OCW2, 0x62);	/* IRQ-02PIC0 */
	data = in_byte(PORT_KEYDAT);
	fifo8_put(&mousefifo, data);

	return;
}

void init_mouse_cursor8(char *mouse, char bc)
{
	static char cursor[16][16] = {
		"**************..",
		"*OOOOOOOOOOO*...",
		"*OOOOOOOOOO*....",
		"*OOOOOOOOO*.....",
		"*OOOOOOOO*......",
		"*OOOOOOO*.......",
		"*OOOOOOO*.......",
		"*OOOOOOOO*......",
		"*OOOO**OOO*.....",
		"*OOO*..*OOO*....",
		"*OO*....*OOO*...",
		"*O*......*OOO*..",
		"**........*OOO*.",
		"*..........*OOO*",
		"............*OO*",
		".............***"
	};
	int x, y;

	for (y = 0; y < 16; y++) {
		for (x = 0; x < 16; x++) {
			if (cursor[y][x] == '*') {
				mouse[y * 16 + x] = COL8_000000;
			}
			if (cursor[y][x] == 'O') {
				mouse[y * 16 + x] = COL8_FFFFFF;
			}
			if (cursor[y][x] == '.') {
				mouse[y * 16 + x] = bc;
			}
		}
	}
	return;
}

int mouse_decode(struct MOUSE_DEC *mdec, unsigned char dat)
{
	if (mdec->phase == 0) {

		if (dat == 0xfa) {
			mdec->phase = 1;
		}
		return 0;
	}
	if (mdec->phase == 1) {

		if ((dat & 0xc8) == 0x08) {

			mdec->buf[0] = dat;
			mdec->phase = 2;
		}
		return 0;
	}
	if (mdec->phase == 2) {

		mdec->buf[1] = dat;
		mdec->phase = 3;
		return 0;
	}
	if (mdec->phase == 3) {

		mdec->buf[2] = dat;
		mdec->phase = 1;
		mdec->btn = mdec->buf[0] & 0x07;
		mdec->x = mdec->buf[1];
		mdec->y = mdec->buf[2];
		if ((mdec->buf[0] & 0x10) != 0) {
			mdec->x |= 0xffffff00;
		}
		if ((mdec->buf[0] & 0x20) != 0) {
			mdec->y |= 0xffffff00;
		}
		mdec->y = - mdec->y;
		return 1;
	}
	return -1;
}

void changeIntIntoCharArr(char *m, int x)
{
	for (int i = 0;i < 4;i++) 
	{
		*(m + i) = ' ';
	}
	if (x == 0) 
	{
		*(m + 3) = '0';
		return;
	}
	int fu = 0;
	if (x < 0) {
		fu = 1;
		x *= -1;
	}
	for (int i = 3;i >= 0;i--) 
	{
		*(m + i) = x % 10 + '0';
		x /= 10;
		if (x == 0)
	   	{
			if (fu == 1)
			{
				*(m + i - 1) = '-';
			}
			return;
		}
	}
}

void mergeMouseLcrMsg(char *s, char *p, int x, int y) 
{
	char xchar[4], ychar[4];
	changeIntIntoCharArr(xchar, x);
	changeIntIntoCharArr(ychar, y);

	for (int i = 0;i < 15;i++) 
	{
		*s = *p;
		if(5 <= i && i <= 8)
		{
			*s = xchar[i - 5];
		}
		if (10 <= i && i <= 13)
		{
			*s = ychar[i - 10];
		}
		s++;
		p++;
	}
	*s = '\0';
}

void mergeMouseCoordinateMsg(char *s, char *p, int mx, int my)
{
	char xchar[4], ychar[4];
	changeIntIntoCharArr(xchar, mx);
	changeIntIntoCharArr(ychar, my);

	for (int i = 0;i < 24;i++) 
	{
		*s = *p;
		if(12 <= i && i <= 15)
		{
			*s = xchar[i - 12];
		}
		if (18 <= i && i <= 21)
		{
			*s = ychar[i - 18];
		}
		s++;
		p++;
	}
	*s = '\0';
}
