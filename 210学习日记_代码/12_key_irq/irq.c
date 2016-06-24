#include "lib.h"

#define GPH2CON		(*(volatile unsigned int *)0xE0200C40)
#define GPH2DAT		(*(volatile unsigned int *)0xE0200C44)
#define GPH3CON		(*(volatile unsigned int *)0xE0200C60)
#define GPH3DAT		(*(volatile unsigned int *)0xE0200C64)

#define EXT_INT_2_CON		(*(volatile unsigned int *)0xE0200E08)
#define EXT_INT_3_CON		(*(volatile unsigned int *)0xE0200E0C)
#define EXT_INT_2_MASK		(*(volatile unsigned int *)0xE0200F08)
#define EXT_INT_3_MASK		(*(volatile unsigned int *)0xE0200F0C)
#define VIC0INTSELECT		(*(volatile unsigned int *)0xF200000C)
#define VIC0INTENABLE		(*(volatile unsigned int *)0xF2000010)
#define VIC0VECTADDR16		(*(volatile unsigned int *)0xF2000140)
#define VIC0ADDRESS  		(*(volatile unsigned int *)0xF2000F00)
#define EXT_INT_2_PEND		(*(volatile unsigned int *)0xE0200F48)
#define EXT_INT_3_PEND		(*(volatile unsigned int *)0xE0200F4c)

int i = 0;

void do_irq(void)
{
	/* ���ж� */
	EXT_INT_2_PEND |= 1<<3;
	EXT_INT_3_PEND |= 1<<0;
	/* ���ж����� */
	VIC0ADDRESS = 0;

	if(!(GPH2DAT & (1<<3)))
	{
		wy_printf("counter(k4) : %d \n",i++);
	}
	if(!(GPH3DAT & (1<<0)))
	{
		wy_printf("counter(K5) : %d \n",i--);
	}
}

extern void key_IRQ(void);
void irq_init(void)
{
	/* ����GPH2_3(K4),GPH3_0(K5)�����ж� */
	GPH2CON |= 0xf<<12;
	GPH3CON |= 0xf<<0;
	/* ���ô�����ʽΪ�½��ش��� */
	EXT_INT_2_CON |= 0x2<<12;
	EXT_INT_3_CON |= 0x2<<0;
	/* ʹ���ж�(GPIO�����) */
	EXT_INT_2_MASK &= ~(1<<3);
	EXT_INT_3_MASK &= ~(1<<0);
	/* ����ΪIRQ�ж� */
	VIC0INTSELECT &= ~(1<<16);
	/* ʹ���ж�(�жϿ����������) */
	VIC0INTENABLE |= 1<<16;
	/* �����ж����� */
	VIC0VECTADDR16 = (int)key_IRQ;
}

