#include "windows.h"
#include "idt.h"

void cstart(void) 
{
	initIdt();
	desktop();
}

void desktop(void)
{
	struct BOOTINFO *binfo = (struct BOOTINFO *) BOOTBASE;
	init_palette();
	init_screen(binfo->vram, binfo->scrnx, binfo->scrny);
	putfonts8_asc(binfo->vram, binfo->scrnx, 30, 60, COL8_FFFFFF, "RKMF OS");
	line(binfo->vram, binfo->scrnx, 80, COL8_FFFFFF);
}
