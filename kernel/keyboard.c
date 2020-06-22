#include "fifo.h"

struct	FIFO8	keyfifo;

#define PIC0_OCW2				0x0020
#define PORT_KEYDAT				0x0060
#define PORT_KEYCMD				0x0064

#define PORT_KEYSTA				0x0064

#define KEYSTA_SEND_NOTREADY	0x02
#define KEYCMD_WRITE_MODE		0x60
#define KBC_MODE				0x47

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
		if ((out_byte(PORT_KEYSTA) & KEYSTA_SEND_NOTREADY) == 0) {
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
