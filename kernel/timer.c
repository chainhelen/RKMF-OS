#include "timer.h"
#include "windows.h"

#define PIT_CTRL 	0x0043
#define PIT_CNT0	0x0040

TIMERCTL	timerctl;

#define TIMER_FLAGS_ALLOC 	1
#define TIMER_FLAGS_USING	2

static void changeIntIntoCharArr(char *m, int x)
{
	for (int i = 0;i < 8;i++) 
	{
		*(m + i) = ' ';
	}
	if (x == 0) 
	{
		*(m + 7) = '0';
		return;
	}
	int fu = 0;
	if (x < 0) {
		fu = 1;
		x *= -1;
	}
	for (int i = 7;i >= 0;i--) 
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

void mergeTimerMsg(char *s, char *p, int x) 
{
	char xchar[8];
	changeIntIntoCharArr(xchar, x);

	for (int i = 0;i < 14;i++) 
	{
		*s = *p;
		if(6 <= i && i <= 13)
		{
			*s = xchar[i - 6];
		}
		s++;
		p++;
	}
	*s = '\0';
}

void init_pit(void)
{
	int i;
	TIMER *t;
	out_byte(PIT_CTRL, 0x34);
	out_byte(PIT_CNT0, 0x9c);
	out_byte(PIT_CNT0, 0x2e);

	timerctl.count = 0;
	for(i = 0;i < MAX_TIMER;i++) {
		timerctl.timers0[i].flags = 0;
	}
	t = timer_alloc();
	t->timeout = 0xffffffff;
	t->flags = TIMER_FLAGS_USING;
	t->next = 0;
	timerctl.t0 = t;
	timerctl.next = 0xffffffff;

	return;
}

TIMER *timer_alloc(void)
{
	int i;
	for (i = 0;i < MAX_TIMER;i++) {
		if (timerctl.timers0[i].flags == 0) {
			timerctl.timers0[i].flags = TIMER_FLAGS_ALLOC;
			return &timerctl.timers0[i];
		}
	}
	return 0;
}

void timer_free(TIMER *timer)
{
	timer->flags = 0;
	return;
}

void timer_init(TIMER *timer,struct FIFO8 *fifo, int data)
{
	timer->fifo = fifo;
	timer->data = data;
	return;
}

void timer_settime(TIMER *timer, unsigned int timeout)
{
	int e;
	TIMER *t, *s;
	timer->timeout = timeout + timerctl.count;
	timer->flags = TIMER_FLAGS_USING;
	e = io_load_eflags();
	io_cli();
	t = timerctl.t0;
	if (timer->timeout <= t->timeout) 
	{
		// struct BOOTINFO *binfo = (struct BOOTINFO *) BOOTBASE;
		// char s[40];
		// mergeTimerMsg(s, "TIME  00000000", t->timeout);
		// putfonts8_asc(binfo->vram, binfo->scrnx, 340, 60, COL8_000000, s);

		timerctl.t0 = timer;
		timer->next = t;
		timerctl.next = timer->timeout;
		io_store_eflags(e);
		return;
	}
	for (;;) {
		s = t;
		t = t->next;
		if (timer->timeout <= t->timeout) {
			s->next = timer;
			timer->next = t;
			io_store_eflags(e);
			return;
		}
	}
}

void timer_handler()
{
	TIMER *timer;
	char ts = 0;
	out_byte(PIC0_OCW2, 0x60);
	timerctl.count++;
	if (timerctl.next > timerctl.count) {
		return;
	}
	timer = timerctl.t0;
	for (;;) {
		if (timer->timeout > timerctl.count) {
			break;
		}


		timer->flags = TIMER_FLAGS_ALLOC;
		if (timer != task_timer) {
			fifo8_put(timer->fifo, timer->data);
		} else {
			ts = 1;
		}
		timer = timer->next;
	}
	timerctl.t0 = timer;
	timerctl.next = timer->timeout;
	if (ts != 0){
		struct BOOTINFO *binfo = (struct BOOTINFO *) 0x6000;
		char s[40];
		mergeTimerMsg(s, "TASK  00000000", 1);
		putfonts8_asc(binfo->vram, binfo->scrnx, 540, 60, COL8_000000, s);
		// task_switch(); 
	}
	return;
}

int timer_cancel(TIMER *timer)
{
	int e;
	TIMER *t;
	e = io_load_eflags();
	io_cli();
	if (timer->flags == TIMER_FLAGS_USING) {
		if (timer == timerctl.t0) {
			t = timer->next;
			timerctl.t0 = t;
			timerctl.next = t->timeout;
		} else {
			t = timerctl.t0;
			for (;;) {
				if (t->next = timer) {
					break;
				}
				t = t->next;
			}
			t->next = timer->next;
		}
		timer->flags = TIMER_FLAGS_ALLOC;
		io_store_eflags(e);
		return 1;
	}
	io_store_eflags(e);
	return 0;
}

void timer_cancelall(struct FIFO8 *fifo)
{
	int e, i;
	TIMER *t;
	e = io_load_eflags();
	io_cli();
	for (i = 0; i < MAX_TIMER; i++) {
		t = &timerctl.timers0[i];
		if (t->flags != 0 && t->fifo == fifo) {
			timer_cancel(t);
			timer_free(t);
		}
	}
	io_store_eflags(e);
	return;
}
