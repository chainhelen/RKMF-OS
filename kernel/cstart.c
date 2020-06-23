#include "windows.h"
#include "idt.h"

#define   Displayaddr       0xa0000
#define   xsize             320
#define   ysize             200

void cstart(void) 
{
	initIdt();
	desktop();
}

void desktop(void)
{
	init_palette();
	init_screen((char *)Displayaddr, xsize, ysize);
	putfonts8_asc((char *)Displayaddr, xsize, 10, 40, COL8_FFFFFF, "RKMF OS");
	line((char *)Displayaddr, xsize, 60, 7);
}
