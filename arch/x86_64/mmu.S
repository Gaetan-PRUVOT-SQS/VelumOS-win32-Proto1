.intel_syntax noprefix
.text

.globl mmu_invlpg
.type mmu_invlpg, @function
mmu_invlpg:
	invlpg [rdi]
	ret

.globl mmu_write_cr3
.type mmu_write_cr3, @function
mmu_write_cr3:
	mov cr3, rdi
	ret

.globl mmu_read_cr2
.type mmu_read_cr2, @function
mmu_read_cr2:
	mov rax, cr2
	ret

.globl mmu_flush_global
.type mmu_flush_global, @function
mmu_flush_global:
	mov rax, cr4
	test eax, 0x80
	jz 1f
	mov rdx, rax
	and rdx, -129
	mov cr4, rdx
	mov cr4, rax
	ret
1:
	mov rax, cr3
	mov cr3, rax
	ret

.globl mmu_gdt_base
.type mmu_gdt_base, @function
mmu_gdt_base:
	sub rsp, 16
	sgdt [rsp]
	mov rax, [rsp + 2]
	add rsp, 16
	ret

.globl mmu_cpuid_edx
.type mmu_cpuid_edx, @function
mmu_cpuid_edx:
	push rbx
	mov eax, edi
	xor ecx, ecx
	cpuid
	mov eax, edx
	pop rbx
	ret

.globl mmu_image_layout
.type mmu_image_layout, @function
mmu_image_layout:
	lea rax, [rip + __kernel_start]
	mov [rdi], rax
	lea rax, [rip + __text_end]
	mov [rdi + 8], rax
	lea rax, [rip + __rodata_start]
	mov [rdi + 16], rax
	lea rax, [rip + __rodata_end]
	mov [rdi + 24], rax
	lea rax, [rip + __data_start]
	mov [rdi + 32], rax
	lea rax, [rip + __kernel_end]
	mov [rdi + 40], rax
	ret

.section .note.GNU-stack,"",@progbits
