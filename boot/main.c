/*#include "multiboot.h"
unsigned char *videobuf = (unsigned char *)0xb8000;
*/

/* 参考 https://www.zhihu.com/question/49580321 */
/*
void print(char *s, int len);

void bp_main(unsigned long magic, multiboot_info_t *mbi)
{
    print("hello world", 11);
    while(1){};
}

void print(char *s, int len)
{
    int i ;
    for (i = 0;i < len;i++)
    {
        videobuf[i * 2 + 0] = s[i];
        videobuf[i * 2 + 1] = 0x17;
    }
    for (; i < 80 * 25;i++)
    {
        videobuf[i * 2 + 0] = ' ';
        videobuf[i * 2 + 1] = 0x17;
    }
}
*/

//the_cpu为每个cpu的数据，目前仅用于cpu栈
char the_cpu[4096] __attribute__((section(".percpu")));
void bp_main(void)
{
  char *w="Hello World!";
  short *p=(short *)0xb8000;
  int i;
  for (i=0;i<12;i++) {
    *p++=w[i]|0x700;
  }
  for(;;);
}
