#include <linux/module.h>
#include <linux/types.h>
#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/string.h>
#include <linux/ioport.h>
#include <linux/platform_device.h>
#include <linux/delay.h>
#include <linux/err.h>
#include <linux/slab.h>
#include <linux/clk.h>
#include <linux/cpufreq.h>

#include <linux/mtd/mtd.h>
#include <linux/mtd/nand.h>
#include <linux/mtd/nand_ecc.h>
#include <linux/mtd/partitions.h>

#include <asm/io.h>

#include <plat/regs-nand.h>
#include <plat/nand.h>

struct nand_regs {
	unsigned long nfconf;
	unsigned long nfcont;
	unsigned long nfcmmd;
	unsigned long nfaddr;
	unsigned long nfdata;
	unsigned long nfmeccd0;
	unsigned long nfmeccd1;
	unsigned long nfseccd;
	unsigned long nfsblk;
	unsigned long nfeblk;
	unsigned long nfstat;
	unsigned long nfeccerr0;
	unsigned long nfeccerr1;
};

static struct nand_regs *nand_regs;
static struct nand_chip *tiny_nand_chip;
static struct mtd_info *tiny_nand_mtd;

static void tiny_nand_select_chip(struct mtd_info *mtd, int chipnr)
{
	if(chipnr == -1)
	{
		/*ȡ��ѡ��*/
		nand_regs->nfcont |= (1<<1);
	}
	else
	{
		/*ѡ��оƬ*/
		nand_regs->nfcont &= ~(1<<1);
	}
}

static void tiny_nand_cmd_ctrl(struct mtd_info *mtd, int dat,
				unsigned int ctrl)
{

	if (ctrl & NAND_CLE)
	{
		/*������*/
		nand_regs->nfcmmd = dat;
	}
	else
	{
		/*����ַ*/
		nand_regs->nfaddr = dat;
	}
}

static int tiny_nand_dev_ready(struct mtd_info *mtd, struct nand_chip *chip)
{
	/*�ȴ�����Ĳ������*/
	return (nand_regs->nfstat & (1<<0));
}

static int tiny_nand_init(void)
{
	struct clk *nand_clk;
	/*1.����һ��nand_chip�ṹ��*/
	tiny_nand_chip = kzalloc(sizeof(struct nand_chip),GFP_KERNEL);
	nand_regs = ioremap(0xB0E00000,sizeof(struct nand_regs));

	/*2.����*/
	/*
	 * ��ʼ��nand_chip�ṹ���еĺ���ָ��
	 * �ṩѡ��оƬ�����������ַ�������ݣ�д���ݣ��ȴ��Ȳ���
	 */
	tiny_nand_chip->select_chip    = tiny_nand_select_chip;
	tiny_nand_chip->cmd_ctrl        = tiny_nand_cmd_ctrl;
	tiny_nand_chip->IO_ADDR_R   = &nand_regs->nfdata;
	tiny_nand_chip->IO_ADDR_W  = &nand_regs->nfdata;
	tiny_nand_chip->dev_ready     = tiny_nand_dev_ready;
	tiny_nand_chip->ecc.mode      = NAND_ECC_SOFT;
	/*3.Ӳ�����*/
	/*ʹ��ʱ��*/
	nand_clk = clk_get(NULL, "nand");
	clk_enable(nand_clk);

	/*
	 * AddrCycle[1]:1 = ���͵�ַ��Ҫ5������
	 */
	nand_regs->nfconf |= 1<<1;
#define TWRPH1    1
#define TWRPH0    1
#define TACLS        1
	nand_regs->nfconf |= (TACLS<<12) | (TWRPH0<<8) | (TWRPH1<<4);
	/*
	 * MODE[0]:1     = ʹ��Nand Flash������
	 * Reg_nCE0[1]:1 = ȡ��Ƭѡ
	 */
	nand_regs->nfcont |= (1<<1)|(1<<0);
	/*4.ʹ��*/
	tiny_nand_mtd = kzalloc(sizeof(struct mtd_info), GFP_KERNEL);
	tiny_nand_mtd->owner = THIS_MODULE;
	tiny_nand_mtd->priv = tiny_nand_chip;

	nand_scan(tiny_nand_mtd, 1);
}

static void tiny_nand_exit(void)
{
	kfree(tiny_nand_mtd);
	iounmap(nand_regs);
	kfree(tiny_nand_chip);
}

module_init(tiny_nand_init);
module_exit(tiny_nand_exit);

MODULE_LICENSE("GPL");

