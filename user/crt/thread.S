.intel_syntax noprefix
.text

.globl __velum_thread_entry
.type __velum_thread_entry, @function
__velum_thread_entry:
	xor ebp, ebp
	and rsp, -16
	call __velum_thread_main
	ud2
.size __velum_thread_entry, . - __velum_thread_entry

.section .note.GNU-stack,"",@progbits
