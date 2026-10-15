.intel_syntax noprefix
.text

.globl v_tcb
.type v_tcb, @function
v_tcb:
	mov rax, qword ptr fs:[0]
	ret
.size v_tcb, . - v_tcb

.globl __velum_errno
.type __velum_errno, @function
__velum_errno:
	mov rax, qword ptr fs:[0]
	add rax, 8
	ret
.size __velum_errno, . - __velum_errno

.section .note.GNU-stack,"",@progbits
