.intel_syntax noprefix
.text

.globl _start
.type _start, @function
_start:
	xor ebp, ebp
	mov rdi, rsp
	and rsp, -16
	call __velum_start
	ud2
.size _start, . - _start

.globl v_relro_start
.type v_relro_start, @function
v_relro_start:
	lea rax, [rip + __relro_start]
	ret
.size v_relro_start, . - v_relro_start

.globl v_relro_end
.type v_relro_end, @function
v_relro_end:
	lea rax, [rip + __relro_end]
	ret
.size v_relro_end, . - v_relro_end

.section .note.GNU-stack,"",@progbits
