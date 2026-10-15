.intel_syntax noprefix
.text

.set SEL_KCODE, 0x08
.set SEL_KDATA, 0x10
.set MXCSR_DEFAULT, 0x1f80

.macro FUNC name
	.globl \name
	.type \name, @function
\name:
.endm

FUNC gdt_load
	lgdt [rdi]
	push SEL_KCODE
	lea rax, [rip + 1f]
	push rax
	retfq
1:
	mov ax, SEL_KDATA
	mov ds, ax
	mov es, ax
	mov ss, ax
	xor eax, eax
	mov fs, ax
	mov gs, ax
	ret

FUNC tss_load
	ltr di
	ret

FUNC idt_load
	lidt [rdi]
	ret

FUNC cpu_read_cr0
	mov rax, cr0
	ret

FUNC cpu_write_cr0
	mov cr0, rdi
	ret

FUNC cpu_read_cr2
	mov rax, cr2
	ret

FUNC cpu_read_cr3
	mov rax, cr3
	ret

FUNC cpu_write_cr3
	mov cr3, rdi
	ret

FUNC cpu_read_cr4
	mov rax, cr4
	ret

FUNC cpu_write_cr4
	mov cr4, rdi
	ret

FUNC cpu_invlpg
	invlpg [rdi]
	ret

FUNC cpu_read_cs
	mov ax, cs
	ret

FUNC cpu_read_ss
	mov ax, ss
	ret

FUNC cpu_read_tr
	str ax
	ret

FUNC cpu_self
	mov rax, gs:[0]
	ret

FUNC cpu_halt
	hlt
	ret

FUNC cpu_wbinvd
	wbinvd
	ret

FUNC cpu_cpuid
	push rbx
	mov eax, edi
	mov ecx, esi
	mov r8, rdx
	cpuid
	mov [r8], eax
	mov [r8 + 4], ebx
	mov [r8 + 8], ecx
	mov [r8 + 12], edx
	pop rbx
	ret

FUNC cpu_xsetbv
	mov ecx, edi
	mov rax, rsi
	mov rdx, rsi
	shr rdx, 32
	xsetbv
	ret

FUNC cpu_fpu_hw_reset
	fninit
	push MXCSR_DEFAULT
	ldmxcsr [rsp]
	add rsp, 8
	ret

FUNC fpu_xsave
	mov eax, -1
	mov edx, -1
	xsave64 [rdi]
	ret

FUNC fpu_xrstor
	mov eax, -1
	mov edx, -1
	xrstor64 [rdi]
	ret

FUNC fpu_fxsave
	fxsave64 [rdi]
	ret

FUNC fpu_fxrstor
	fxrstor64 [rdi]
	ret

FUNC cpu_trigger_int3
	int3
	ret

FUNC cpu_trigger_test_vec
	int 0x2f
	ret

FUNC cpu_boot_stack_low
	lea rax, [rip + boot_stack_bottom]
	ret

.section .note.GNU-stack,"",@progbits
