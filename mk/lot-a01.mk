LOTS += a01
KSRC_a01 := $(sort $(wildcard arch/x86_64/cpu/*.c)) \
	arch/x86_64/cpu/isr.S arch/x86_64/cpu/cpu_asm.S arch/x86_64/cpu/cpu_fault.S
DIRS_a01 := arch/x86_64/cpu tests/host/a01 include/velum
HT_a01 := $(sort $(wildcard tests/host/a01/test_*.c))
HSRC_a01 := arch/x86_64/cpu/gdt_encode.c arch/x86_64/cpu/cpuid_collect.c \
	arch/x86_64/cpu/cpuid_decode.c arch/x86_64/cpu/cpuid_brand.c \
	arch/x86_64/cpu/cpuid_table.c arch/x86_64/cpu/trap_policy.c \
	arch/x86_64/cpu/trap_names.c arch/x86_64/cpu/idt_table.c \
	arch/x86_64/cpu/backtrace.c lib/libk/str.c lib/libk/str2.c \
	lib/libk/memcmp.c
HINC_a01 := -Iarch/x86_64/cpu
