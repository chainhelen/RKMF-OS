void asmfunc1(void);
void asmfunc2(void);
void asmfunc3(void);
void asmfunc4(void);


void cstart(void)
{
	asmfunc1();
	asmfunc2();
	asmfunc3();
	asmfunc4();

	while (1)
	{;}
}
