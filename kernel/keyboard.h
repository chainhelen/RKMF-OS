#ifndef KEYBOARD_H
#define KEYBOARD_H

#define PIC0_OCW2				0x0020
#define PORT_KEYDAT				0x0060
#define PORT_KEYCMD				0x0064

#define PORT_KEYSTA				0x0064

#define KEYSTA_SEND_NOTREADY	0x02
#define KEYCMD_WRITE_MODE		0x60
#define KBC_MODE				0x47

void keyboardhandler(void);
void wait_KBC_sendready(void);
void init_keyboard(void);
void mergeKeyboardMsg(char *s, char *p, int data);

#endif
