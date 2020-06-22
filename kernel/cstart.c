#include "windows.h"
#include "idt.h"

#define   Displayaddr       0xa0000
#define   xsize             320
#define   ysize             200

void cstart(void) 
{
	init_palette();
	init_screen((char *)Displayaddr, xsize, ysize);
	// initIdt();
}
