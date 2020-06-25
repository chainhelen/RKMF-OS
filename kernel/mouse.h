#ifndef MOUSE_H
#define MOUSE_H

struct MOUSE_DEC {
	unsigned char buf[3], phase;
	int x, y, btn;
};


void enable_mouse(struct MOUSE_DEC *mdec);
void init_mouse_cursor8(char *mouse, char bc);
int mouse_decode(struct MOUSE_DEC *mdec, unsigned char dat);

#define PORT_KEYDAT				0x0060
#define PORT_KEYCMD				0x0064
#define KEYCMD_SENDTO_MOUSE		0xd4
#define MOUSECMD_ENABLE			0xf4

#define PIC0_OCW2		0x0020
#define PIC1_OCW2		0x00a0

#endif
