// GPIO
#define GPICON  	(*(volatile unsigned int *)0xE0200220)	//IIS Signals

// IIS
#define IISCON  	(*(volatile unsigned int *)0xEEE30000)	//IIS Control
#define IISMOD  	(*(volatile unsigned int *)0xEEE30004)	//IIS Mode
#define IISFIC  	(*(volatile unsigned int *)0xEEE30008)	//IIS FIFO Control
#define IISPSR  	(*(volatile unsigned int *)0xEEE3000C)	//IIS Prescaler
#define IISTXD		(*(volatile unsigned int *)0xEEE30010)	//IIS TXD DATA
#define IISRXD 		(*(volatile unsigned int *)0xEEE30014)	//IIS RXD DATA
#define IISFICS  	(*(volatile unsigned int *)0xEEE30018)	//IIS FIFO Control

//CLCK
#define EPLL_CON0  	(*(volatile unsigned int *)0xe0100110)
#define EPLL_CON1  	(*(volatile unsigned int *)0xe0100114)
#define CLK_SRC0  	(*(volatile unsigned int *)0xE0100200)		
#define CLK_CON  	(*(volatile unsigned int *)0xEEE10000)	

void IIS_init(void)
{
	/* ���ö�ӦGPIO����IIS */
	GPICON = 0x22222222;

	/* �������໷
	 * SDIV [2:0]  : SDIV = 0x3
	 * PDIV [13:8] : PDIV = 0x3
	 * MDIV [24:16]: MDIV = 0x43
	 * LOCKED  [29]: 1 = ʹ����
	 * ENABLE  [31]: 1 = ʹ�����໷
	 *
	 * Fout = (0x43+0.7)*24M / (3 * 2^3) = 67.7*24M/24 = 67.7Mhz
	 */
	EPLL_CON0 = 0xa8430303;	/* MPLL_FOUT = 67.7Mhz */
	EPLL_CON1 = 0xbcee;          /* K = 0xbcee */

	/* ʱ��Դ������
	 * APLL_SEL[0] :1 = FOUTAPLL
	 * MPLL_SEL[4] :1 = FOUTMPLL
	 * EPLL_SEL[8] :1 = FOUTEPLL
	 * VPLL_SEL[12]:1 = FOUTVPLL
	 * MUX_MSYS_SEL[16]:0 = SCLKAPLL
	 * MUX_DSYS_SEL[20]:0 = SCLKMPLL
	 * MUX_PSYS_SEL[24]:0 = SCLKMPLL
	 * ONENAND_SEL [28]:1 = HCLK_DSYS
	 */	
	CLK_SRC0 = 0x10001111;

	/* ʱ��Դ�Ľ�һ������(AUDIO SUBSYSTEMCLK SRC)
	 * bit[3:2]: 00 = MUXi2s_a_out��Դ��Main CLK
	 * bit[0]  : 1 = Main CLK��Դ��FOUT_EPLL
	 */
	CLK_CON = 0x1;
	/* ����AUDIO SUBSYSTEMCLK DIV�Ĵ���ʹ�õ���Ĭ��ֵ���ʷ�Ƶϵ��Ϊ1 */
			
	// IISCDCLK  11.289Mhz = 44.1K * 256fs 
	// IISSCLK    1.4112Mhz = 44.1K * 32fs
	// IISLRCLK   44.1Khz
	/* Ԥ��Ƶֵ
	 * bit[13:8] : N = 5
	 * bit[15]   : ʹ��Ԥ��Ƶ
	 */
	IISPSR = 1<<15 | 5<<8;
	
	/* ����IIS������
	 * bit[0]: 1 = ʹ��IIS
	 */
	IISCON |= 1<<0 | (unsigned)1<<31;
	
	/* ���ø���ʱ�����
	 * bit[2:1]:IISSCLK(λʱ��)  44.1K * 32fs = 1.4112Mhz
	 * bit[3:4]:IISCDCLK(ϵͳʱ��) 44.1K * 256fs = 11.289Mhz
	 * bit[9:8]:10 = �ȿ��Է����ֿ��Խ���
	 * bit[10] :0 = PCLK is internal source clock for IIS 
	 */
	IISMOD = 1<<9 | 0<<8 | 1<<10;
}
