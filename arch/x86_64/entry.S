.intel_syntax noprefix
.text

.globl _start
.type _start, @function
_start:
	cli
	cld
	lea rsp, [rip + boot_stack_top]
	xor ebp, ebp
	call boot_guard_seed
	mov [rip + __stack_chk_guard], rax
	call kmain
2:
	cli
	hlt
	jmp 2b

.section .bss
.balign 4096
.globl boot_stack_guard
boot_stack_guard:
	.skip 4096
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
