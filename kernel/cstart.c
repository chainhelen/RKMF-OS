void cstart(void) 
{
	squareness(0,0,199,319,0); /* win1 */
	squareness(30,90,90,150,1); /* win1 */
	squareness(30,170,90,230,2);/* win2 */
	squareness(110,90,170,150,3);/* win3 */
	squareness(110,170,170,230,4);/* win4 */

	while (1)
	{;}
}

void drawpoint(int x,int y,int color)     /* 画点 */
{
	*(char *)(0xa0000+320*x+y) =color;
}

void squareness(int startx,int starty,int endx,int endy,int color)  /* 画矩形 */
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
