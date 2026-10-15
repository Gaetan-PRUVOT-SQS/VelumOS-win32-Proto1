.intel_syntax noprefix
.text

.globl v_syscall6
.type v_syscall6, @function
v_syscall6:
	mov rax, rdi
	mov r11, rsi
	mov rdi, [r11]
	mov rsi, [r11 + 8]
	mov rdx, [r11 + 16]
	mov r10, [r11 + 24]
	mov r8, [r11 + 32]
	mov r9, [r11 + 40]
	syscall
	ret
.size v_syscall6, . - v_syscall6

.section .note.GNU-stack,"",@progbits
