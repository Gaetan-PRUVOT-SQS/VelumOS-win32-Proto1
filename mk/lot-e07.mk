LOTS += e07
KSRC_e07 := $(sort $(wildcard arch/x86_64/smp/*.c arch/x86_64/smp/*.S))
DIRS_e07 := arch/x86_64/smp tests/host/e07
HT_e07 := $(sort $(wildcard tests/host/e07/test_*.c))
HSRC_e07 := arch/x86_64/smp/smp_plan.c arch/x86_64/smp/smp_tramp.c \
	arch/x86_64/apic/lapic_icr.c lib/libk/str.c lib/libk/str2.c lib/libk/memcmp.c
HINC_e07 := -Iarch/x86_64/smp -Iarch/x86_64/apic
