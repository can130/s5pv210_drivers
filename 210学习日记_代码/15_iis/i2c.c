#include "lib.h"

/* GPIO */
#define GPD1CON		(*(volatile unsigned int *)0xE02000C0)
#define GPD1PUD		(*(volatile unsigned int *)0xE02000C8)

/* IIC */
#define IICCON		(*(volatile unsigned int *)0xE1800000)
#define IICSTAT    	(*(volatile unsigned int *)0xE1800004)
#define IICDS		(*(volatile unsigned int *)0xE180000C)

#define VIC0ADDRESS  		(*(volatile unsigned int *)0xF2000F00)
#define VIC1ADDRESS  		(*(volatile unsigned int *)0xF2100F00)
#define VIC2ADDRESS  		(*(volatile unsigned int *)0xF2200F00)
#define VIC3ADDRESS  		(*(volatile unsigned int *)0xF2300F00)

void Delay(int time);

#define WRDATA      (1)
#define RDDATA      (2)

typedef struct tI2C {
    unsigned char *pData;   /* ���ݻ����� */
    volatile int DataCount; /* �ȴ���������ݳ��� */
    volatile int Status;    /* ״̬ */
    volatile int Mode;      /* ģʽ����/д */
    volatile int Pt;        /* pData�д��������ݵ�λ�� */
}t210_I2C, *pt210_I2C;

static t210_I2C g_t210_I2C;

void i2c_init(void)
{
	/* ѡ�����Ź��ܣ�GPE15:IICSDA, GPE14:IICSCL */
	GPD1CON |= 0x22;
	GPD1PUD |= 0x5;

	/* bit[7] = 1, ʹ��ACK
	* bit[6] = 0, IICCLK = PCLK/16
	* bit[5] = 1, ʹ���ж�
	* bit[3:0] = 0xf, Tx clock = IICCLK/16
	* PCLK = 66.7MHz, IICCLK = 4.1MHz
	*/
	IICCON = (1<<7) | (0<<6) | (1<<5) | (0xf);  // 0xaf

	IICSTAT = 0x10;     // I2C�������ʹ��(Rx/Tx)
}

/*
 * ��������
 * slvAddr : �ӻ���ַ��buf : ���ݴ�ŵĻ�������len : ���ݳ��� 
 */
void i2c_write(unsigned int slvAddr, unsigned char *buf, int len)
{
    g_t210_I2C.Mode = WRDATA;   // д����
    g_t210_I2C.Pt   = 0;        // ����ֵ��ʼΪ0
    g_t210_I2C.pData = buf;     // ���滺������ַ
    g_t210_I2C.DataCount = len; // ���䳤��
    
    IICDS   = slvAddr;
    IICSTAT = 0xf0;         // �������ͣ�����
    
    /* �ȴ�ֱ�����ݴ������ */    
    while (g_t210_I2C.DataCount != -1);
}
        
/*
 * ��������
 * slvAddr : �ӻ���ַ��buf : ���ݴ�ŵĻ�������len : ���ݳ��� 
 */
void i2c_read(unsigned int slvAddr, unsigned char *buf, int len)
{
    g_t210_I2C.Mode = RDDATA;   // ������
    g_t210_I2C.Pt   = -1;       // ����ֵ��ʼ��Ϊ-1����ʾ��1���ж�ʱ����������(��ַ�ж�)
    g_t210_I2C.pData = buf;     // ���滺������ַ
    g_t210_I2C.DataCount = len; // ���䳤��
    
    IICDS        = slvAddr;
    IICSTAT      = 0xb0;    // �������գ�����
    
    /* �ȴ�ֱ�����ݴ������ */    
    while (g_t210_I2C.DataCount != 0);
}

//----------IIC�жϷ�����----------
void do_irq(void) 
{
	unsigned int iicSt,i;

	wy_printf("do_irq \n");
	
	iicSt  = IICSTAT; 

	if(iicSt & 0x8){ wy_printf("Bus arbitration failed\n"); }

	switch (g_t210_I2C.Mode)
	{    
		case WRDATA:
		{
			if((g_t210_I2C.DataCount--) == 0)
			{
				// �������������ָ�I2C����������P�ź�
				IICSTAT = 0xd0;
				IICCON  = 0xaf;
				Delay(10000);  // �ȴ�һ��ʱ���Ա�P�ź��Ѿ�����
				break;    
			}

			IICDS = g_t210_I2C.pData[g_t210_I2C.Pt++];

			// ������д��IICDS����Ҫһ��ʱ����ܳ�����SDA����
			for (i = 0; i < 10; i++);   

			IICCON = 0xaf;      // �ָ�I2C����
			break;
		}

		case RDDATA:
		{
			if (g_t210_I2C.Pt == -1)
			{
				// ����ж��Ƿ���I2C�豸��ַ�����ģ�û������
				// ֻ����һ������ʱ����Ҫ����ACK�ź�
				g_t210_I2C.Pt = 0;
				if(g_t210_I2C.DataCount == 1)
					IICCON = 0x2f;   // �ָ�I2C���䣬��ʼ�������ݣ����յ�����ʱ������ACK
				else 
					IICCON = 0xaf;   // �ָ�I2C���䣬��ʼ��������
				break;
			}

			g_t210_I2C.pData[g_t210_I2C.Pt++] = IICDS;
			g_t210_I2C.DataCount--;

			if (g_t210_I2C.DataCount == 0)
			{

				// �������лָ�I2C����������P�ź�
				IICSTAT = 0x90;
				IICCON  = 0xaf;
				Delay(10000);  // �ȴ�һ��ʱ���Ա�P�ź��Ѿ�����
				break;    
			}      
			else
			{           
				// �������һ������ʱ����Ҫ����ACK�ź�
				if(g_t210_I2C.DataCount == 1)
					IICCON = 0x2f;   // �ָ�I2C���䣬���յ���һ����ʱ��ACK
				else 
					IICCON = 0xaf;   // �ָ�I2C���䣬���յ���һ����ʱ����ACK
			}
			break;
		}

		default:
		    break;      
	}
	// ���ж�����
	VIC0ADDRESS = 0x0;
	VIC1ADDRESS = 0x0;
	VIC2ADDRESS = 0x0;
	VIC3ADDRESS = 0x0;
} 

/*
 * ��ʱ����
 */
void Delay(int time)
{
	for (; time > 0; time--);
}

unsigned char at24cxx_read(unsigned char address)
{
	unsigned char val;
	wy_printf("at24cxx_read address = %d\r\n", address);
	i2c_write(0xA0, &address, 1);
	wy_printf("at24cxx_read send address ok\r\n");
	i2c_read(0xA0, (unsigned char *)&val, 1);
	wy_printf("at24cxx_read get data ok\r\n");
	return val;
}

void at24cxx_write(unsigned char address, unsigned char data)
{
	unsigned char val[2];
	val[0] = address;
	val[1] = data;
	i2c_write(0xA0, val, 2);
}

void wm8960_write(unsigned int slave_addr, int addr, int data)
{
	unsigned char val[2];
	val[0] = addr<<1 | ((data>>8) & 0x0001);
	val[1] = (data & 0x00FF);
	i2c_write(slave_addr, val, 2);
}

