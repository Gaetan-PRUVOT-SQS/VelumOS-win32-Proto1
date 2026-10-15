.intel_syntax noprefix
.text

.globl irq_save
.type irq_save, @function
irq_save:
	pushfq
	pop rax
	cli
	ret

.globl irq_restore
.type irq_restore, @function
irq_restore:
	test edi, 0x200
	jz 1f
	sti
1:
	ret

.globl irq_disable
.type irq_disable, @function
irq_disable:
	cli
	ret

.globl irq_enable
.type irq_enable, @function
irq_enable:
	sti
	ret

.globl cpu_relax
.type cpu_relax, @function
cpu_relax:
	pause
	ret

.section .note.GNU-stack,"",@progbits
