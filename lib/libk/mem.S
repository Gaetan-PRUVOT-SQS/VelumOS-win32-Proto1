.intel_syntax noprefix
.text

.globl memcpy
.type memcpy, @function
memcpy:
	mov rax, rdi
	mov rcx, rdx
	rep movsb
	ret

.globl memmove
.type memmove, @function
memmove:
	mov rax, rdi
	mov rcx, rdx
	cmp rdi, rsi
	jbe 1f
	lea r8, [rsi + rdx]
	cmp rdi, r8
	jae 1f
	lea rdi, [rdi + rdx - 1]
	lea rsi, [rsi + rdx - 1]
	std
	rep movsb
	cld
	ret
1:
	rep movsb
	ret

.globl memset
.type memset, @function
memset:
	mov r8, rdi
	mov eax, esi
	mov rcx, rdx
	rep stosb
	mov rax, r8
	ret

.section .note.GNU-stack,"",@progbits
