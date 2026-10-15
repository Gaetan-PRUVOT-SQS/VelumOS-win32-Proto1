.intel_syntax noprefix
.text

.globl _start
.type _start, @function
_start:
	cli
	cld
	lea rsp, [rip + boot_stack_top]
	xor ebp, ebp
	rdtsc
	shl rdx, 32
	or rax, rdx
	mov r8, rax
	mov eax, 1
	cpuid
	bt ecx, 30
	jnc 1f
	rdrand rax
	xor r8, rax
1:
	mov rcx, 0x9e3779b97f4a7c15
	imul r8, rcx
	and r8, -256
	mov [rip + __stack_chk_guard], r8
	call kmain
2:
	cli
	hlt
	jmp 2b

.section .bss
.balign 16
.globl boot_stack_bottom
boot_stack_bottom:
	.skip 32768
.globl boot_stack_top
boot_stack_top:

.data
.balign 8
.globl __stack_chk_guard
__stack_chk_guard:
	.quad 0x595e9fbd94fda766

.section .note.GNU-stack,"",@progbits
