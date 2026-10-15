.intel_syntax noprefix
.text

.globl syscall_entry
.type syscall_entry, @function
syscall_entry:
	swapgs
	mov gs:[16], rsp
	mov rsp, gs:[8]
	push 0x1b
	push qword ptr gs:[16]
	push r11
	push 0x23
	push rcx
	push 0
	push 0x100
	push rax
	push rbx
	push rcx
	push rdx
	push rsi
	push rdi
	push rbp
	push r8
	push r9
	push r10
	push r11
	push r12
	push r13
	push r14
	push r15
	xor ebp, ebp
	sti
	mov rdi, rsp
	call syscall_entry_c
	cli
	test eax, eax
	pop r15
	pop r14
	pop r13
	pop r12
	pop r11
	pop r10
	pop r9
	pop r8
	pop rbp
	pop rdi
	pop rsi
	pop rdx
	pop rcx
	pop rbx
	pop rax
	lea rsp, [rsp + 16]
	jz 1f
	swapgs
	mov rsp, [rsp + 24]
	sysretq
1:
	swapgs
	iretq
.size syscall_entry, . - syscall_entry

.section .note.GNU-stack,"",@progbits
