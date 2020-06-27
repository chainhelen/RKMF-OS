#include "task.h"
#include "memory.h"
#include "windows.h"
#include "gdt.h"

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


TASK *task_init(struct MEMMAN *memman)
{
	int i;
	TASK *task;
	struct SEGMENT_DESCRIPTOR *gdt = (struct SEGMENT_DESCRIPTOR *) ADR_GDT;
	taskctl = (struct TASKCTL *) memman_alloc_4k(memman, sizeof (TASKCTL));
	for (i = 0; i < MAX_TASKS; i++) {
		taskctl->tasks0[i].flags = 0;
		taskctl->tasks0[i].sel = (TASK_GDT0 + i) * 8;
		set_segmdesc(gdt + TASK_GDT0 + i, 103, (int) &taskctl->tasks0[i].tss, AR_TSS32);
	}
	task = task_alloc();
	task->flags = 2; /* 活动中标志 */
	taskctl->running = 1;
	taskctl->now = 0;
	taskctl->tasks[0] = task;
	load_tr(task->sel);
	task_timer = timer_alloc();
	timer_settime(task_timer, 2);
	return task;
}

TASK *task_alloc(void)
{
	int i;
	TASK *task;
	for (i = 0; i < MAX_TASKS; i++) {
		if (taskctl->tasks0[i].flags == 0) { /* 找到一个未在使用 */
			task = &taskctl->tasks0[i];
			task->flags = 1; /* 正在使用的标志 */
			task->tss.eflags = 0x00000202; /* IF = 1; */
			task->tss.eax = 0; /* 这里先置为 0 */
			task->tss.ecx = 0;
			task->tss.edx = 0;
			task->tss.ebx = 0;
			task->tss.ebp = 0;
			task->tss.esi = 0;
			task->tss.edi = 0;
			task->tss.es = 0;
			task->tss.ds = 0;
			task->tss.fs = 0;
			task->tss.gs = 0;
			task->tss.ldtr = 0;
			task->tss.iomap = 0x40000000;
			return task;
		}
	}
	return 0; /* 全部正在使用 */
}

void task_run(TASK *task)
{
	task->flags = 2; /* 活动中标志 */
	taskctl->tasks[taskctl->running] = task;
	taskctl->running++;
	return;
}

void task_switch(void)
{
	timer_settime(task_timer, 2);
	if (taskctl->running >= 2) {
		taskctl->now++;
		if (taskctl->now == taskctl->running) {
			taskctl->now = 0;
		}

		farjmp(0, taskctl->tasks[taskctl->now]->sel);
	}
	return;
}




void task_sleep(TASK *task)
{
	int i;
	char ts = 0;
	if (task->flags == 2) {		/* 如果指定任务处于唤醒状态 */
		if (task == taskctl->tasks[taskctl->now]) {
			ts = 1; /* 让自己休眠，稍后需要任务切换 */
		}
		for (i = 0; i < taskctl->running; i++) {
			if (taskctl->tasks[i] == task) {
				break;
			}
		}
		taskctl->running--;
		if (i < taskctl->now) {
			taskctl->now--;
		}
		for (; i < taskctl->running; i++) {
			taskctl->tasks[i] = taskctl->tasks[i + 1];
		}
		task->flags = 1; /* 不工作的状态 */
		if (ts != 0) {
			/* 任务切换 */
			if (taskctl->now >= taskctl->running) {
				taskctl->now = 0;
			}
			farjmp(0, taskctl->tasks[taskctl->now]->sel);
		}
	}
	return;
}
