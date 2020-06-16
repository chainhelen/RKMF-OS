void cstart(void) 
{
	char *str = "Hello RKMF-OS\0";
	for (int i=0;i<13;i++)
	{
		*(char *)(0xb8000+15*160+i*2) = str[i];  
		*(char *)(0xb8000+15*160+i*2+1)= 0x0c;
	}
}
