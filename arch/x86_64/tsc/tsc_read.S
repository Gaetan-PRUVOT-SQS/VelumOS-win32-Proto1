.intel_syntax noprefix
.text

.globl tsc_read
.type tsc_read, @function
tsc_read:
	lfence
	rdtsc
	shl rdx, 32
	or rax, rdx
	ret

.section .note.GNU-stack,"",@progbits
