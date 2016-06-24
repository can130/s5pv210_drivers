#include "command.h"
#include "clock.h"
#include "led.h"
#include "uart.h"
#include "lib.h"
#include "nand.h"
#include "i2c.h"
#include "iis.h"

#define	CFG_PROMPT		"WY_BOOT # "	/* Monitor Command Prompt	*/
#define	CFG_CBSIZE		256		/* Console I/O Buffer Size	*/
#define   WM8960_DEVICE_ADDR		0x34

int offset = 0x2E;				/* wav�ļ�ͷ���Ĵ�С */
short *p = (short *)0x23000000;

char *argv[10];

void WM8960_init(void)
{
	/* ��λ�����������еļĴ����ָ���Ĭ��ֵ */ 
	wm8960_write(WM8960_DEVICE_ADDR, 0xf, 0x0);

	/* �򿪵�Դ��ʹ��fast start-upģʽ */
	wm8960_write(WM8960_DEVICE_ADDR, 0x19, 1<<8 | 1<<7 | 1<<6);
	/* ��Ȼ�Ǵ򿪵�Դ */
	wm8960_write(WM8960_DEVICE_ADDR, 0x1a, 1<<8 | 1<<7 | 1<<6 | 1<<5 | 1<<4 | 1<<3);
	/* �����������ʹ�� */
	wm8960_write(WM8960_DEVICE_ADDR, 0x2F, 1<<3 | 1<<2);
	
	/* ����ʱ�ӣ�ʹ�õĶ���Ĭ��ֵ */
	wm8960_write(WM8960_DEVICE_ADDR, 0x4, 0x0);

	/* �ؼ��ǽ�R5�Ĵ�����bit[3]���㣬�رվ������� */
	wm8960_write(WM8960_DEVICE_ADDR, 0x5, 0x0);
	
	/* ����ͨ��Э�鷽ʽ:��������24λ����IIS����������ʱ�ӵ�ƽ�Ƿ�ת */
	wm8960_write(WM8960_DEVICE_ADDR, 0x7, 0x2);
		
	/* ��������������������� */
	wm8960_write(WM8960_DEVICE_ADDR, 0x2, 0xFF | 0x100);/* ������������ */
	wm8960_write(WM8960_DEVICE_ADDR, 0x3, 0xFF | 0x100);/* ������������ */
	wm8960_write(WM8960_DEVICE_ADDR, 0xa, 0xFF | 0x100);/* ������������ */
	wm8960_write(WM8960_DEVICE_ADDR, 0xb, 0xFF | 0x100);/* ������������ */
	
	/* ʹ��ͨ��������ᾲ�� */
	wm8960_write(WM8960_DEVICE_ADDR, 0x22, 1<<8 | 1<<7);/* ������������ */
	wm8960_write(WM8960_DEVICE_ADDR, 0x25, 1<<8 | 1<<7);	/* ������������ */
	
	return;
}

int readline (const char *const prompt)
{
	char console_buffer[CFG_CBSIZE];		/* console I/O buffer	*/
	char *buf = console_buffer;
	int argc = 0;
	int state = 0;

	//puts(prompt);
	wy_printf("%s",prompt);
	gets(console_buffer);
	
	while (*buf)
	{
		if (*buf != ' ' && state == 0)
		{
			argv[argc++] = buf;
			state = 1;
		}
		
		if (*buf == ' ' && state == 1)
		{
			*buf = '\0';
			state = 0;
		}
		
		buf++;	
	}
	
	return argc;
}

void message(void)
{
	wy_printf("\nThis bootloader support some command to test peripheral:\n");
	wy_printf("Such as: LCD, IIS, BUZZER \n");
	wy_printf("Try 'help' to learn them \n\n");	
}

int main(void)
{
	char buf[6];
	int argc = 0;
	int i = 0;

	led_init(); /* ���ö�Ӧ�ܽ�Ϊ��� */
	uart_init(); /* ��ʼ��UART0 */
	nand_read_id(buf);

	i2c_init(); /* ��ʼ��IIC */
	WM8960_init();
	IIS_init();
	
	wy_printf("\n**********************************************************\n");
	wy_printf("                     wy_bootloader\n");
	wy_printf("                     vars: %d \n",2012);
	wy_printf("                     nand id:");
	putchar_hex(buf[0]);
	putchar_hex(buf[1]);
	putchar_hex(buf[2]);
	putchar_hex(buf[3]);
	putchar_hex(buf[4]);
	wy_printf("\n**********************************************************\n");
	while (1)
	{
		argc = readline (CFG_PROMPT);
		if(argc == 0 && i ==0)
		{
			message();
			i=1;
		}
		run_command(argc, argv);
	}	
	
	return 0;
}

