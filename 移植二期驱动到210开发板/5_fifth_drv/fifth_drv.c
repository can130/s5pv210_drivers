#include <linux/device.h>
#include <linux/interrupt.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/fs.h>
#include <linux/init.h>
#include <linux/delay.h>
#include <linux/irq.h>
#include <asm/uaccess.h>
#include <asm/irq.h>
#include <asm/io.h>
#include <mach/gpio.h>
#include <linux/poll.h>

static struct class *fifthdrv_class;

volatile unsigned long *gph2con;
volatile unsigned long *gph2dat;

volatile unsigned long *gph3con;
volatile unsigned long *gph3dat;

static DECLARE_WAIT_QUEUE_HEAD(button_waitq);

/* �ж��¼���־, �жϷ����������1��fifth_drv_read������0 */
static volatile int ev_press = 0;

static struct fasync_struct *button_async;

struct pin_desc{
	unsigned int pin;
	unsigned int key_val;
};

/* ��ֵ: ����ʱ, 0x01, 0x02, 0x03, 0x04, 0x05 */
/* ��ֵ: �ɿ�ʱ, 0x81, 0x82, 0x83, 0x84, 0x85 */
static unsigned char key_val;

struct pin_desc pins_desc[5] = {
	{S5PV210_GPH2(3), 0x01},
	{S5PV210_GPH3(0), 0x02},
	{S5PV210_GPH3(1), 0x03},
	{S5PV210_GPH3(2), 0x04},
	{S5PV210_GPH3(3), 0x05},
};

/*
  * ȷ������ֵ
  */
static irqreturn_t buttons_irq(int irq, void *dev_id)
{
	struct pin_desc * pindesc = (struct pin_desc *)dev_id;
	unsigned int pinval;
	
	pinval = gpio_get_value(pindesc->pin);

	if (pinval)
	{
		/* �ɿ� */
		key_val = 0x80 | pindesc->key_val;
	}
	else
	{
		/* ���� */
		key_val = pindesc->key_val;
	}

	ev_press = 1;                  /* ��ʾ�жϷ����� */
	wake_up_interruptible(&button_waitq);   /* �������ߵĽ��� */
	
	kill_fasync (&button_async, SIGIO, POLL_IN);
	
	return IRQ_RETVAL(IRQ_HANDLED);
}

static int fifth_drv_open(struct inode *inode, struct file *file)
{
	/* ע���ж� */
	request_irq(IRQ_EINT(19),  buttons_irq, IRQF_TRIGGER_FALLING|IRQF_TRIGGER_RISING, "K4", &pins_desc[0]);
	request_irq(IRQ_EINT(24),  buttons_irq, IRQF_TRIGGER_FALLING|IRQF_TRIGGER_RISING, "K5", &pins_desc[1]);
	request_irq(IRQ_EINT(25), buttons_irq, IRQF_TRIGGER_FALLING|IRQF_TRIGGER_RISING, "K6", &pins_desc[2]);
	request_irq(IRQ_EINT(26), buttons_irq, IRQF_TRIGGER_FALLING|IRQF_TRIGGER_RISING, "K7", &pins_desc[3]);	
	request_irq(IRQ_EINT(27), buttons_irq, IRQF_TRIGGER_FALLING|IRQF_TRIGGER_RISING, "K8", &pins_desc[4]);	

	return 0;
}

ssize_t fifth_drv_read(struct file *file, char __user *buf, size_t size, loff_t *ppos)
{
	if (size != 1)
		return -EINVAL;

	/* ���û�а�������, ���� */
	wait_event_interruptible(button_waitq, ev_press);

	/* ����а�������, ���ؼ�ֵ */
	copy_to_user(buf, &key_val, 1);
	ev_press = 0;
	
	return 1;
}

int fifth_drv_close(struct inode *inode, struct file *file)
{
	free_irq(IRQ_EINT(19), &pins_desc[0]);
	free_irq(IRQ_EINT(24), &pins_desc[1]);
	free_irq(IRQ_EINT(25), &pins_desc[2]);
	free_irq(IRQ_EINT(26), &pins_desc[3]);
	free_irq(IRQ_EINT(27), &pins_desc[4]);
	return 0;
}

static unsigned fifth_drv_poll(struct file *file, poll_table *wait)
{
	unsigned int mask = 0;
	poll_wait(file, &button_waitq, wait); // ������������

	if (ev_press)
		mask |= POLLIN | POLLRDNORM;

	return mask;
}

static int fifth_drv_fasync (int fd, struct file *filp, int on)
{
	printk("driver: fifth_drv_fasync\n");
	return fasync_helper (fd, filp, on, &button_async);
}

static struct file_operations sencod_drv_fops = {
	.owner   =  THIS_MODULE,    /* ����һ���꣬�������ģ��ʱ�Զ�������__this_module���� */
	.open    =  fifth_drv_open,     
	.read	 =	fifth_drv_read,	   
	.release =  fifth_drv_close,
	.poll    =  fifth_drv_poll,
	.fasync	 =  fifth_drv_fasync,
};

int major;
static int fifth_drv_init(void)
{
	major = register_chrdev(0, "fifth_drv", &sencod_drv_fops);

	fifthdrv_class = class_create(THIS_MODULE, "fifth_drv");

	device_create(fifthdrv_class, NULL, MKDEV(major, 0), NULL, "buttons"); /* /dev/buttons */

	gph2con = (volatile unsigned long *)ioremap(0xe0200c40, 16);
	gph2dat = gph2con + 1;

	gph3con = (volatile unsigned long *)ioremap(0xE0200C60, 16);
	gph3dat = gph3con + 1;

	return 0;
}

static void fifth_drv_exit(void)
{
	unregister_chrdev(major, "fifth_drv");
	device_destroy(fifthdrv_class, MKDEV(major, 0));
	class_destroy(fifthdrv_class);
	iounmap(gph2con);
	iounmap(gph3con);
	return 0;
}

module_init(fifth_drv_init);
module_exit(fifth_drv_exit);

MODULE_LICENSE("GPL");

