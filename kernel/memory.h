#ifndef MEMORY_H
#define MEMORY_H

#define MEMMAN_FREES 4000
#define MEMMAN_ADDR  0x00200000

//内存片
//起始地址和大小
typedef struct _FREEINFO{
	unsigned int *addr, size;
}FREEINFO;

//内存管理器
typedef struct _MEMMAN{
	int frees, maxfrees, lostsize, losts;
	FREEINFO free[MEMMAN_FREES];
}MEMMAN;

//初始化内存管理器
void memman_init(MEMMAN *man);
//内存总共可用量
unsigned int memman_total(MEMMAN *man);
int memman_free(MEMMAN *man, unsigned int *addr, unsigned int size);
unsigned int* memman_alloc(MEMMAN *man, unsigned int size);
void mergeMemMsg(char *s, char *p, int total, int free);

#endif
