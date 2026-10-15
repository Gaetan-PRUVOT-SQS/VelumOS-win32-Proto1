.intel_syntax noprefix
.text

.globl sched_switch_stacks
.type sched_switch_stacks, @function
sched_switch_stacks:
	push rbp
	push rbx
	push r12
	push r13
	push r14
	push r15
	mov [rdi], rsp
	mov rsp, rsi
	pop r15
	pop r14
	pop r13
	pop r12
	pop rbx
	pop rbp
	ret
.size sched_switch_stacks, . - sched_switch_stacks

.globl sched_thread_trampoline
.type sched_thread_trampoline, @function
sched_thread_trampoline:
	mov rdi, r12
	xor ebp, ebp
	and rsp, -16
	call sched_thread_entry
	ud2
.size sched_thread_trampoline, . - sched_thread_trampoline

.globl arch_enter_user
.type arch_enter_user, @function
arch_enter_user:
	cli
	push 0x1b
	push rsi
	push 0x202
	push 0x23
	push rdi
	mov rdi, rdx
	xor eax, eax
	xor ebx, ebx
	xor ecx, ecx
	xor edx, edx
	xor esi, esi
	xor ebp, ebp
	xor r8d, r8d
	xor r9d, r9d
	xor r10d, r10d
	xor r11d, r11d
	xor r12d, r12d
	xor r13d, r13d
	xor r14d, r14d
	xor r15d, r15d
	swapgs
	iretq
.size arch_enter_user, . - arch_enter_user

.globl arch_idle_wait
.type arch_idle_wait, @function
arch_idle_wait:
	sti
	hlt
	ret
.size arch_idle_wait, . - arch_idle_wait

.globl arch_irqs_enabled
.type arch_irqs_enabled, @function
arch_irqs_enabled:
	pushfq
	pop rax
	and eax, 0x200
	ret
.size arch_irqs_enabled, . - arch_irqs_enabled

.globl arch_boot_stack_bottom
.type arch_boot_stack_bottom, @function
arch_boot_stack_bottom:
	lea rax, [rip + boot_stack_bottom]
	ret
.size arch_boot_stack_bottom, . - arch_boot_stack_bottom

.globl arch_boot_stack_top
.type arch_boot_stack_top, @function
arch_boot_stack_top:
	lea rax, [rip + boot_stack_top]
	ret
.size arch_boot_stack_top, . - arch_boot_stack_top

.section .note.GNU-stack,"",@progbits
