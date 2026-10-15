#define v_tcb v_tcb_real
#define __velum_errno velum_errno_real
#include "../../../user/crt/tls.S"

.intel_syntax noprefix
.text

.globl tls_probe
.type tls_probe, @function
tls_probe:
	push rbx
	push r12
	push r13
	sub rsp, 16
	mov rbx, rdi
	mov r12, rsi
	mov eax, 158
	mov edi, 0x1003
	mov rsi, rsp
	syscall
	mov eax, 158
	mov edi, 0x1002
	mov rsi, rbx
	syscall
	call v_tcb_real
	mov [r12], rax
	call velum_errno_real
	mov [r12 + 8], rax
	mov eax, 158
	mov edi, 0x1002
	mov rsi, [rsp]
	syscall
	add rsp, 16
	pop r13
	pop r12
	pop rbx
	ret
.size tls_probe, . - tls_probe

.section .note.GNU-stack,"",@progbits
