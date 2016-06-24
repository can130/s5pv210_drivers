
.global _start
	
_start:
	ldr sp, =0xD0030000  /* ��ʼ��ջ����Ϊ����Ҫ����C���� */
	bl clock_init              /* ��ʼ��ʱ�� */
	bl ddr_init                   /* ��ʼ���ڴ� */
	bl nand_init               /* ��ʼ��NAND */

	ldr r0, =0x36000000   /* Ҫ������DDR�е�λ�� */
	ldr r1, =0x0                 /* ��NAND��0��ַ��ʼ���� */
	ldr r2, =bss_start         /* BSS�εĿ�ʼ��ַ */
	sub r2,r2,r0                  /* Ҫ�����Ĵ�С */
	bl nand_read              /* �������� */

clean_bss:
	ldr r0, =bss_start
	ldr r1, =bss_end
	mov r3, #0
	cmp r0, r1
	ldreq pc, =on_ddr
clean_loop:
	str r3, [r0], #4
	cmp r0, r1	
	bne clean_loop		
	ldr pc, =on_ddr

on_ddr:
	mrs  r0, cpsr
	bic	r0,r0,#0x1f  /* ��M4~M0 */
	orr	r0,r0,#0x12
	msr	cpsr,r0        /* ����irq */
	ldr sp, =0x3e000000    /* ��ʼ����ͨ�ж�ģʽ��ջ��ָ���ڴ� */

	bl irq_init

	mrs  r0, cpsr
	bic	r0,r0,#0x9f  /* ���ܵ��жϿ���,��M4~M0 */
	orr	r0,r0,#0x10
	msr	cpsr,r0      /* ����user mode */

	ldr sp, =0x3f000000    /* ��ʼ���û�ģʽ��ջ��ָ���ڴ� */
	ldr pc, =main

halt:
	b halt	

.global key_IRQ

key_IRQ:
	sub lr, lr, #4                   /* 1.���㷵�ص�ַ */
	stmdb sp!, {r0-r12, lr}  /* 2.�����ֳ� */

	/* 3. �����쳣 */
	bl do_irq
	
	/* 4. �ָ��ֳ� */
	ldmia sp!, {r0-r12, pc}^  /* ^��ʾ��spsr�ָ���cpsr */

