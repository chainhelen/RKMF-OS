#include "fifo.h"
#include "keyboard.h"
#include "windows.h"

struct	FIFO8	keyfifo;
#define BOOTBASE      0x6000

void mergeKeyboardMsg(char *s, char *p, int data) 
{
	for (; *p != '\0';p++) 
	{
		*s = *p;
		s++;
	}

	int x = data / 16;
	int y = data % 16;

	char *mp = "0123456789ABCDEF";

	*s = mp[x];
	s++;
	*s = mp[y];
	s++;

	*s = '\0';
}

void keyboardhandler(void)
{
	unsigned char data;
	out_byte(PIC0_OCW2, 0x61);	 
	data = in_byte(PORT_KEYDAT);
	fifo8_put(&keyfifo, data);
}

void wait_KBC_sendready(void)
{

	for (;;) {
		 if ((in_byte(PORT_KEYSTA) & KEYSTA_SEND_NOTREADY) == 0) {
			break;
		 }
	}
	return;
}

void init_keyboard(void)
{
	wait_KBC_sendready();
	out_byte(PORT_KEYCMD, KEYCMD_WRITE_MODE);
	wait_KBC_sendready();
	out_byte(PORT_KEYDAT, KBC_MODE);
	return;
}

