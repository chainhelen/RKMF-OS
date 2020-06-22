#ifndef FIFO8_H
#define FIFO8_H

struct FIFO8 {
	unsigned char *buf;
	int p, q, size, free, flags;
};

#define FLAGS_OVERRUN		0x0001

#endif
