#include "smp_int.h"

.section .rodata
.balign 16
.globl smp_tramp_start
smp_tramp_start:
.code16
	cli
	cld
	movw %cs, %ax
	movw %ax, %ds
	movzwl %ax, %ebx
	shll $4, %ebx
	lgdtl TD_GDTR
	movl %cr0, %eax
	orl $1, %eax
	movl %eax, %cr0
	ljmpl *TD_FAR32
.code32
.globl smp_tramp_pm32
smp_tramp_pm32:
	movw $TRAMP_SEL_DATA, %ax
	movw %ax, %ds
	movw %ax, %es
	movw %ax, %ss
	movl %cr4, %eax
	orl $0x20, %eax
	movl %eax, %cr4
	movl TD_CR3(%ebx), %eax
	movl %eax, %cr3
	movl $0xc0000080, %ecx
	rdmsr
	orl $0x900, %eax
	wrmsr
	movl %cr0, %eax
	orl $0x80000000, %eax
	movl %eax, %cr0
	ljmp *TD_FAR64(%ebx)
.code64
.globl smp_tramp_lm64
smp_tramp_lm64:
	movl %ebx, %ebx
	movq TD_STACK(%rbx), %rsp
	movq TD_ARG(%rbx), %rdi
	movq TD_ENTRY(%rbx), %rax
	xorl %ebp, %ebp
	jmp *%rax
.globl smp_tramp_end
smp_tramp_end:

.section .note.GNU-stack,"",@progbits
