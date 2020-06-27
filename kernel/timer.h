#include "fifo.h"

#ifndef TIMER_H
#define TIMER_H

#define PIC0_OCW2		0x0020
#define MAX_TIMER		500
typedef struct _TIMER {
	struct TIMER *next;
	unsigned int timeout, flags;
	struct FIFO8 	*fifo;
	int data;
} TIMER;

typedef struct _TIMERCTL {
	unsigned int count, next, using;
	TIMER *t0;
	TIMER timers0[MAX_TIMER];
} TIMERCTL;


TIMER *timer_alloc(void);
void timer_init(TIMER *timer,struct FIFO8 *fifo, int data);
extern TIMER *task_timer;	

#endif
