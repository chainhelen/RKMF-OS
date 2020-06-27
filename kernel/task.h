#include "timer.h"

#ifndef TASK_H
#define TASK_H

#define MAX_TASKS		2  	/* 最大任务数量 */
#define TASK_GDT0		6	  	/* 定义从 GDT 的几号开始分配给 TSS */
#define AR_TSS32		0x0089

typedef struct _TSS32 {
	int backlink, esp0, ss0, esp1, ss1, esp2, ss2, cr3;
	int eip, eflags, eax, ecx, edx, ebx, esp, ebp, esi, edi;
	int es, cs, ss, ds, fs, gs;
	int ldtr, iomap;
}TSS32;

typedef struct _TASK {
	int sel, flags; /* sel 用来存放 GDT 的编号 */
	TSS32 tss;
} TASK;

typedef struct _TASKCTL {
	int running; /* 正在运行的任务数量 */
	int now; /* 用来记录当前正在运行的是哪个任务 */
	TASK *tasks[MAX_TASKS];
	TASK tasks0[MAX_TASKS];
} TASKCTL;


void task_switch(void);
void task_run(TASK *task);
TASK *task_alloc(void);

#define ADR_GDT  0x7e00

TASKCTL *taskctl;
TIMER *task_timer;

#endif
