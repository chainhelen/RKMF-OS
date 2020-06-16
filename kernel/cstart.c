void cstart(void) 
{
	int i=0;
	for (i=0;i<=10;i++)
	{
		*(char *)(0xb8000+23*160+i) = 'C';  
		*(char *)(0xb8000+23*160+1+i)= 0x0c;
		i++;
	}
}
