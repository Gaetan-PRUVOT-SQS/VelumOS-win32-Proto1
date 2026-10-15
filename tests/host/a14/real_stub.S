#define v_syscall6 v_syscall6_real
#include "../../../user/crt/syscall.S"

.intel_syntax noprefix
.text

.globl stub_preserve_check
.type stub_preserve_check, @function
stub_preserve_check:
	push rbx
	push rbp
	push r12
	push r13
	push r14
	push r15
	sub rsp, 56
	mov qword ptr [rsp], 0
	mov qword ptr [rsp + 8], 0
	mov qword ptr [rsp + 16], 0
	mov qword ptr [rsp + 24], 0
	mov qword ptr [rsp + 32], 0
	mov qword ptr [rsp + 40], 0
	mov rbx, 0x1111111111111111
	mov rbp, 0x2222222222222222
	mov r12, 0x3333333333333333
	mov r13, 0x4444444444444444
	mov r14, 0x5555555555555555
	mov r15, 0x6666666666666666
	mov edi, 39
	mov rsi, rsp
	call v_syscall6_real
	mov r8, 0x1111111111111111
	cmp rbx, r8
	jne .Lregister_changed
	mov r8, 0x2222222222222222
	cmp rbp, r8
	jne .Lregister_changed
	mov r8, 0x3333333333333333
	cmp r12, r8
	jne .Lregister_changed
	mov r8, 0x4444444444444444
	cmp r13, r8
	jne .Lregister_changed
	mov r8, 0x5555555555555555
	cmp r14, r8
	jne .Lregister_changed
	mov r8, 0x6666666666666666
	cmp r15, r8
	jne .Lregister_changed
	xor eax, eax
	jmp .Lpreserve_done
.Lregister_changed:
	mov eax, 1
.Lpreserve_done:
	add rsp, 56
	pop r15
	pop r14
	pop r13
	pop r12
	pop rbp
	pop rbx
	ret
.size stub_preserve_check, . - stub_preserve_check

.section .note.GNU-stack,"",@progbits
