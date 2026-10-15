.intel_syntax noprefix
.text

.globl r3_do_null
.type r3_do_null, @function
r3_do_null:
	xor eax, eax
	mov rax, [rax]
	ud2
.size r3_do_null, . - r3_do_null

.globl r3_do_div0
.type r3_do_div0, @function
r3_do_div0:
	xor ecx, ecx
	mov eax, 1
	xor edx, edx
	div ecx
	ud2
.size r3_do_div0, . - r3_do_div0

.globl r3_thread_tramp
.type r3_thread_tramp, @function
r3_thread_tramp:
	mov rdi, [rsp]
	call r3_thread_main
	ud2
.size r3_thread_tramp, . - r3_thread_tramp

.globl r3_do_overflow
.type r3_do_overflow, @function
r3_do_overflow:
	sub rsp, 4096
	mov [rsp], rax
	jmp r3_do_overflow
.size r3_do_overflow, . - r3_do_overflow

.globl r3_ovf_tramp
.type r3_ovf_tramp, @function
r3_ovf_tramp:
	call r3_ovf_main
	ud2
.size r3_ovf_tramp, . - r3_ovf_tramp

.globl r3_hold_tramp
.type r3_hold_tramp, @function
r3_hold_tramp:
	mov rdi, [rsp]
	call r3_hold_main
	ud2
.size r3_hold_tramp, . - r3_hold_tramp

.section .note.GNU-stack,"",@progbits
