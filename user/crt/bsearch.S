.intel_syntax noprefix
.text

.globl bsearch
.type bsearch, @function
bsearch:
	sub rsp, 56
	mov [rsp], rdi
	mov [rsp + 8], rsi
	mov [rsp + 16], rdx
	mov [rsp + 24], rcx
	mov [rsp + 32], r8
	mov rdi, rsp
	call vbsearch_impl
	add rsp, 56
	ret
.size bsearch, . - bsearch

.section .note.GNU-stack,"",@progbits
