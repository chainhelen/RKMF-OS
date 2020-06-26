#include "memory.h"

int memman_free(MEMMAN *man, unsigned int *addr, unsigned int size){
	int i, j;
	//从空闲内存片中找到第一片大于待释放的内存片大小的首地址
	for(i=0; i<man->frees; i++){
		if(man->free[i].addr>addr){
			break;
		}
	}
	
	//可以往前合并或者同时前后合并
	if(i>0){
		//待释放的内存片刚好可以和前一个内存片合并
		if(man->free[i-1].addr+man->free[i-1].size==addr){
			man->free[i-1].size += size;
			if(i<man->frees){
				//待释放的内存片刚好可以和后一片合并
				//因此总可用内存片数量减一  合并成更大的空闲内存
				if(addr+size==man->free[i].addr){
					man->free[i-1].size += man->free[i].size;
					man->frees--;
				}
			}
			return 0;
		}
	}
	
	//只能向后合并
	if(i<man->frees){
		if(addr+size==man->free[i].addr){
			man->free[i].addr = addr;
			man->free[i].size += size;
			return 0;
		}
	}
	
	//不能和前后合并 
	//如果此时内存片数量没有超过最大值 则新增一个空闲块
	//将释放的内存块放在i位置  i位置的内存块依次后移
	if(man->frees<MEMMAN_FREES){
		for(j=man->frees; j>i; j--){
			man->free[j] = man->free[j-1];
		}
		man->frees++;
		if(man->maxfrees<man->frees){
			man->maxfrees = man->frees;
		}
		man->free[i].addr = addr;
		man->free[i].size = size;
		return 0;
	}
	
	//不能和前后合并 
	//如果此时内存片数量超过最大值 则该释放内存不能再重复利用记录一下
	man->losts++;
	man->lostsize += size;
	return -1;
}


//简单内存分配算法
//从可用内存中找到第一块大于所需内存大小的内存片地址返回
unsigned int* memman_alloc(MEMMAN *man, unsigned int size){
	unsigned int *a;
	for(int i=0; i<man->frees; i++){
		if(man->free[i].size>=size){
			a = man->free[i].addr;
			man->free[i].size -= size;
			man->free[i].addr = (unsigned int *)(((char *)(man->free[i].addr))+size);
			if(man->free[i].size==0){
				for(int j=i+1; j<man->frees; j++){
					man->free[j-1] = man->free[j];
				}
				man->frees--;
			}
			return a;
		}
	}
	return (unsigned int*)0;
}

void memman_init(MEMMAN *man)
{
       man->frees = 0;       //还没有一块可用内存
       man->maxfrees = 0;
       man->losts = 0;       //释放失败的次数为0
       man->lostsize = 0;  //释放失败的总内存大小也为0
}

unsigned int memman_total(MEMMAN *man)
{
       unsigned int i,t=0;
       for(i=0;i < man->frees; i++)
      {
              t+= man->free[i].size;    //只考虑所有可用内存占得总内存大小，不考虑内存连续（肯定有不连续的）。
      }
      return t;
}

static void changeIntIntoCharArr(char *m, int x)
{
	for (int i = 0;i < 4;i++) 
	{
		*(m + i) = ' ';
	}
	if (x == 0) 
	{
		*(m + 3) = '0';
		return;
	}
	int fu = 0;
	if (x < 0) {
		fu = 1;
		x *= -1;
	}
	for (int i = 3;i >= 0;i--) 
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

void mergeMemMsg(char *s, char *p, int total, int free) 
{
	char xchar[4], ychar[4];
	changeIntIntoCharArr(xchar, total);
	changeIntIntoCharArr(ychar, free);

	for (int i = 0;i < 29;i++) 
	{
		*s = *p;
		if(9 <= i && i <= 12)
		{
			*s = xchar[i - 9];
		}
		if (23 <= i && i <= 26)
		{
			*s = ychar[i - 23];
		}
		s++;
		p++;
	}
	*s = '\0';
}

unsigned int memman_alloc_4k(struct MEMMAN *man, unsigned int size)
{
   unsigned int a;
   size = (size + 0xfff) & 0xfffff000;
   a = memman_alloc(man, size);
   return a;
}

int memman_free_4k(struct MEMMAN *man, unsigned int addr, unsigned int size)
{
	int i;
	size = (size + 0x3fff) & 0xffffC000; // actually this is memman_free_16k
	i = memman_free(man, addr, size);
	return i;
}
