
.global _start
	
_start:
	ldr sp, =0xD0030000	@��ʼ����ջ
	bl clock_init       @��ʼ��ʱ��
	bl ddr_init         @��ʼ����ջ
	
	b main

