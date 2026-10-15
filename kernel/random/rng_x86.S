.intel_syntax noprefix
.text

.globl rng_x86_rdtsc
.type rng_x86_rdtsc, @function
rng_x86_rdtsc:
	rdtsc
	shl rdx, 32
	or rax, rdx
	ret

.globl rng_x86_rdrand
.type rng_x86_rdrand, @function
rng_x86_rdrand:
	rdrand rax
	jnc 1f
	mov [rdi], rax
	mov eax, 1
	ret
1:
	xor eax, eax
	ret

.globl rng_x86_rdseed
.type rng_x86_rdseed, @function
rng_x86_rdseed:
	rdseed rax
	jnc 1f
	mov [rdi], rax
	mov eax, 1
	ret
1:
	xor eax, eax
	ret

.section .note.GNU-stack,"",@progbits
