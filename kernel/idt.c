#include "idt.h"
#include "font.h"

/* 门描述符 */
typedef struct s_gate
{
	u16	offset_low;	/* Offset Low */
	u16	selector;	/* Selector */
	u8	dcount;		/* 该字段只在调用门描述符中有效。如果在利用
				   调用门调用子程序时引起特权级的转换和堆栈
				   的改变，需要将外层堆栈中的参数复制到内层
				   堆栈。该双字计数字段就是用于说明这种情况
				   发生时，要复制的双字参数的数量。*/
	u8	attr;		/* P(1) DPL(2) DT(1) TYPE(4) */
	u16	offset_high;	/* Offset High */
}GATE;


u8		idt_ptr[6];	/* 0~15:Limit  16~47:Base */
GATE   	idt[IDT_SIZE];

void init_idt() 
{
	u16* p_idt_limit = (u16*)(&idt_ptr[0]);
	u32* p_idt_base  = (u32*)(&idt_ptr[2]);
	*p_idt_limit = IDT_SIZE * sizeof(GATE) - 1;
	*p_idt_base  = (u32)&idt;

	init_8259A();
	init_idt_all_esc();
}

void init_8259A() 
{
	/* Master 8259, ICW1. */
	out_byte(0x20,	0x11);

	/* Slave  8259, ICW1. */
	out_byte(0xA0,	0x11);

	/* Master 8259, ICW2. 设置 '主8259' 的中断入口地址为 0x20. */
	out_byte(0x21,	0x20);

	/* Slave  8259, ICW2. 设置 '从8259' 的中断入口地址为 0x28 */
	out_byte(0xA1,	0x28);

	/* Master 8259, ICW3. IR2 对应 '从8259'. */
	out_byte(0x21,	0x4);

	/* Slave  8259, ICW3. 对应 '主8259' 的 IR2. */
	out_byte(0xA1,	0x2);

	/* Master 8259, ICW4. */
	out_byte(0x21,	0x1);

	/* Slave  8259, ICW4. */
	out_byte(0xA1,	0x1);

	// 这个地方是个坑，1代表代表关闭，0代表开启
	/* Master 8259, OCW1.  */
	// out_byte(0x21,	0xF9);
	out_byte(0x21,	0xF8);

	/* Slave  8259, OCW1.  */
	// out_byte(0xA1,	0xEF);
	out_byte(0xA1,	0xEF);
}
/* 中断处理函数 */
void	divide_error();
void	single_step_exception();
void	nmi();
void	breakpoint_exception();
void	overflow();
void	bounds_check();
void	inval_opcode();
void	copr_not_available();
void	double_fault();
void	copr_seg_overrun();
void	inval_tss();
void	segment_not_present();
void	stack_exception();
void	general_protection();
void	page_fault();
void	copr_error();
// void    hwint00();
void 	asm_timer_handler();
// void    hwint01();
void   	asm_keyboardhandler();
void    hwint02();
void    hwint03();
void    hwint04();
void    hwint05();
void    hwint06();
void    hwint07();
void    hwint08();
void    hwint09();
void    hwint10();
void    hwint11();
// void    hwint12();
void    asm_mousehandler();
void    hwint13();
void    hwint14();
void    hwint15();

static void init_idt_desc(unsigned char vector, u8 desc_type,
			  int_handler handler, unsigned char privilege);
void init_idt_all_esc()
{
	init_idt_desc(INT_VECTOR_DIVIDE,	DA_386IGate,
		      divide_error,		PRIVILEGE_KRNL);

	init_idt_desc(INT_VECTOR_DEBUG,		DA_386IGate,
		      single_step_exception,	PRIVILEGE_KRNL);

	init_idt_desc(INT_VECTOR_NMI,		DA_386IGate,
		      nmi,			PRIVILEGE_KRNL);

	init_idt_desc(INT_VECTOR_BREAKPOINT,	DA_386IGate,
		      breakpoint_exception,	PRIVILEGE_USER);

	init_idt_desc(INT_VECTOR_OVERFLOW,	DA_386IGate,
		      overflow,			PRIVILEGE_USER);

	init_idt_desc(INT_VECTOR_BOUNDS,	DA_386IGate,
		      bounds_check,		PRIVILEGE_KRNL);

	init_idt_desc(INT_VECTOR_INVAL_OP,	DA_386IGate,
		      inval_opcode,		PRIVILEGE_KRNL);

	init_idt_desc(INT_VECTOR_COPROC_NOT,	DA_386IGate,
		      copr_not_available,	PRIVILEGE_KRNL);

	init_idt_desc(INT_VECTOR_DOUBLE_FAULT,	DA_386IGate,
		      double_fault,		PRIVILEGE_KRNL);

	init_idt_desc(INT_VECTOR_COPROC_SEG,	DA_386IGate,
		      copr_seg_overrun,		PRIVILEGE_KRNL);

	init_idt_desc(INT_VECTOR_INVAL_TSS,	DA_386IGate,
		      inval_tss,		PRIVILEGE_KRNL);

	init_idt_desc(INT_VECTOR_SEG_NOT,	DA_386IGate,
		      segment_not_present,	PRIVILEGE_KRNL);

	init_idt_desc(INT_VECTOR_STACK_FAULT,	DA_386IGate,
		      stack_exception,		PRIVILEGE_KRNL);

	init_idt_desc(INT_VECTOR_PROTECTION,	DA_386IGate,
		      general_protection,	PRIVILEGE_KRNL);

	init_idt_desc(INT_VECTOR_PAGE_FAULT,	DA_386IGate,
		      page_fault,		PRIVILEGE_KRNL);

	init_idt_desc(INT_VECTOR_COPROC_ERR,	DA_386IGate,
		      copr_error,		PRIVILEGE_KRNL);

//	init_idt_desc(INT_VECTOR_IRQ0 + 0,      DA_386IGate,
//			hwint00,                  PRIVILEGE_KRNL);

	init_idt_desc(INT_VECTOR_IRQ0 + 0,      DA_386IGate,
			asm_timer_handler,                  PRIVILEGE_KRNL);

	// 键盘中断
	// init_idt_desc(INT_VECTOR_IRQ0 + 1,      DA_386IGate,
	// 		hwint01,                  PRIVILEGE_KRNL);
	init_idt_desc(INT_VECTOR_IRQ0 + 1,      DA_386IGate,
			asm_keyboardhandler,      PRIVILEGE_KRNL);

	init_idt_desc(INT_VECTOR_IRQ0 + 2,      DA_386IGate,
			hwint02,                  PRIVILEGE_KRNL);

	init_idt_desc(INT_VECTOR_IRQ0 + 3,      DA_386IGate,
			hwint03,                  PRIVILEGE_KRNL);

	init_idt_desc(INT_VECTOR_IRQ0 + 4,      DA_386IGate,
			hwint04,                  PRIVILEGE_KRNL);

	init_idt_desc(INT_VECTOR_IRQ0 + 5,      DA_386IGate,
			hwint05,                  PRIVILEGE_KRNL);

	init_idt_desc(INT_VECTOR_IRQ0 + 6,      DA_386IGate,
			hwint06,                  PRIVILEGE_KRNL);

	init_idt_desc(INT_VECTOR_IRQ0 + 7,      DA_386IGate,
			hwint07,                  PRIVILEGE_KRNL);

	init_idt_desc(INT_VECTOR_IRQ8 + 0,      DA_386IGate,
			hwint08,                  PRIVILEGE_KRNL);

	init_idt_desc(INT_VECTOR_IRQ8 + 1,      DA_386IGate,
			hwint09,                  PRIVILEGE_KRNL);

	init_idt_desc(INT_VECTOR_IRQ8 + 2,      DA_386IGate,
			hwint10,                  PRIVILEGE_KRNL);

	init_idt_desc(INT_VECTOR_IRQ8 + 3,      DA_386IGate,
			hwint11,                  PRIVILEGE_KRNL);

	// init_idt_desc(INT_VECTOR_IRQ8 + 4,      DA_386IGate,
	// 		hwint12,                  PRIVILEGE_KRNL);
	init_idt_desc(INT_VECTOR_IRQ8 + 4,      DA_386IGate,
	 		asm_mousehandler,             PRIVILEGE_KRNL);

	init_idt_desc(INT_VECTOR_IRQ8 + 5,      DA_386IGate,
			hwint13,                  PRIVILEGE_KRNL);

	init_idt_desc(INT_VECTOR_IRQ8 + 6,      DA_386IGate,
			hwint14,                  PRIVILEGE_KRNL);

	init_idt_desc(INT_VECTOR_IRQ8 + 7,      DA_386IGate,
			hwint15,                  PRIVILEGE_KRNL);
}

/*======================================================================*
                             init_idt_desc
 *----------------------------------------------------------------------*
 初始化 386 中断门
 *======================================================================*/
static void init_idt_desc(unsigned char vector, u8 desc_type,
			  int_handler handler, unsigned char privilege)
{
	GATE *	p_gate	= &idt[vector];
	u32	base	= (u32)handler;
	p_gate->offset_low	= base & 0xFFFF;
	p_gate->selector	= 0x08;
	p_gate->dcount		= 0;
	p_gate->attr		= desc_type | (privilege << 5);
	p_gate->offset_high	= (base >> 16) & 0xFFFF;
}

void spurious_irq(int irq)
{
        // disp_str("spurious_irq: ");
        // disp_int(irq);
        // disp_str("\n");
}

