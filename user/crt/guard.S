.intel_syntax noprefix

.data
.balign 8
.globl __stack_chk_guard
.type __stack_chk_guard, @object
__stack_chk_guard:
	.quad 0x595e9fbd94fda700
.size __stack_chk_guard, . - __stack_chk_guard

.text
.globl v_guard_set
.type v_guard_set, @function
v_guard_set:
	mov [rip + __stack_chk_guard], rdi
	ret
.size v_guard_set, . - v_guard_set

.section .note.GNU-stack,"",@progbits
