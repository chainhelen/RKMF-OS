#define   Displayaddr       0xa0000
#define   xsize             320
#define   ysize             200

void cstart(void) 
{
	squareness(0, ysize - 28, xsize -  1, ysize - 28,1); /*任务栏*/
	squareness(0, ysize - 27, xsize -  1, ysize - 27,2);
	squareness(0, ysize - 26, xsize -  1, ysize -  1,1);

	squareness(3, ysize - 24, 59, ysize - 24,2);  /*"开始按钮"*/
	squareness(2, ysize - 24,  2, ysize -  4,2);
	squareness(3, ysize -  4, 59, ysize -  4,3);
	squareness(59,ysize - 23, 59, ysize -  5,3);
	squareness(2, ysize -  3, 59, ysize -  3,4);
	squareness(60,ysize - 24, 60, ysize -  3,4);

	squareness( xsize - 47, ysize - 24, xsize -  4, ysize - 24, 3); /*"时间区域"*/
	squareness( xsize - 47, ysize - 23, xsize - 47, ysize -  4, 3);
	squareness( xsize - 47, ysize -  3, xsize -  4, ysize -  3, 2);
	squareness( xsize -  3, ysize - 24, xsize -  3, ysize -  3, 2);
}

void drawpoint(int x,int y,int color)
{
	*(char *)(Displayaddr+xsize*y+x)=color;
}


void squareness(int startx,int starty,int endx,int endy,int color)
{
	int x,y=0;
	for (y=starty;y<=endy;y++)
	{
		for (x=startx;x<=endx;x++)
		{
			drawpoint(x,y,color);
		}
	}
}
