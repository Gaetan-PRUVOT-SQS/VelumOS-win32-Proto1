.intel_syntax noprefix
.text

.set NONCANONICAL, 0x8000000000000000
.set RECURSE_FRAME, 64

.macro FUNC name
	.globl \name
	.type \name, @function
\name:
.endm

.macro WITNESS
	push rbx
	push r12
	push r15
	movabs rbx, 0x1111111111111111
	movabs r12, 0x1212121212121212
	movabs r15, 0x1515151515151515
.endm

.macro UNWITNESS
	pop r15
	pop r12
	pop rbx
	ret
.endm

FUNC cpu_fault_div0
	WITNESS
	mov eax, 0xa0a0
	xor edx, edx
	xor ecx, ecx
fault_div0_at:
	div rcx
	UNWITNESS

FUNC cpu_fault_gp
	WITNESS
	movabs rax, NONCANONICAL
fault_gp_at:
	mov rax, [rax]
	UNWITNESS

FUNC cpu_fault_ud
	WITNESS
fault_ud_at:
	ud2
	UNWITNESS

FUNC cpu_fault_pf
	WITNESS
fault_pf_at:
	mov rax, [rdi]
	UNWITNESS

FUNC cpu_fault_df
	WITNESS
	mov rax, rsp
	movabs rsp, NONCANONICAL
	push rax
	mov rsp, rax
	UNWITNESS

FUNC cpu_fault_nmi
	WITNESS
	int 2
	UNWITNESS

FUNC cpu_fault_stack
	WITNESS
	call fault_recurse
	UNWITNESS

fault_recurse:
	push rbp
	mov rbp, rsp
	sub rsp, RECURSE_FRAME
	call fault_recurse
	leave
	ret

FUNC cpu_fault_rip
	mov edi, edi
	xor eax, eax
	cmp edi, 3
	ja 1f
	lea rax, [rip + fault_sites]
	mov rax, [rax + rdi * 8]
1:
	ret

.section .rodata
.balign 8
fault_sites:
	.quad fault_div0_at
	.quad fault_gp_at
	.quad fault_ud_at
	.quad fault_pf_at

.section .note.GNU-stack,"",@progbits
