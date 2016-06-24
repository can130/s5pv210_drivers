#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/errno.h>
#include <linux/string.h>
#include <linux/mm.h>
#include <linux/slab.h>
#include <linux/delay.h>
#include <linux/fb.h>
#include <linux/init.h>
#include <linux/dma-mapping.h>
#include <linux/interrupt.h>
#include <linux/platform_device.h>
#include <linux/clk.h>
#include <linux/cpufreq.h>

#include <asm/io.h>
#include <asm/div64.h>

#include <asm/mach/map.h>
#include <mach/regs-gpio.h>

static struct fb_info *tiny_lcd;

static int lcd_init(void)
{
	/* 1. ����һ��fb_info */
	tiny_lcd = framebuffer_alloc(0, NULL);

	/* 2. ���� */
	/* 2.1 ���ù̶��Ĳ��� */
	/* 2.2 ���ÿɱ�Ĳ��� */
	/* 2.3 ���ò������� */
	/* 2.4 ���������� */

	/* 3. Ӳ����صĲ��� */
	/* 3.1 ����GPIO����LCD */
	/* 3.2 ʹ��ʱ�� */
	/* 3.3 ����LCD�ֲ�����LCD������, ����VCLK��Ƶ�ʣ��Դ�� */

	/* 4. ע�� */
	register_framebuffer(tiny_lcd);
	
	return 0;
}

static void lcd_exit(void)
{
}

module_init(lcd_init);
module_exit(lcd_exit);

MODULE_LICENSE("GPL");


