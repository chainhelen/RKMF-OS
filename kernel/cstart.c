#include "windows.h"
#include "idt.h"


void cstart(void) 
{
	drawAllWindows();
	initIdt();
}
