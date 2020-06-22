#ifndef FONT_h
#define FONT_h

extern char hankaku[4096][8];
void putfonts8_asc(char *vram, int xsize, int x, int y, char c, unsigned char *s);
void line(char *vram,int xsize,int y,char c);
void boxfill8(unsigned char *vram, int xsize, unsigned char c, int x0, int y0, int x1, int y1);

#endif
