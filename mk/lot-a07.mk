LOTS += a07
KSRC_a07 := $(sort $(wildcard kernel/proc/*.c)) \
	$(sort $(wildcard arch/x86_64/syscall*.c)) arch/x86_64/syscall_entry.S
DIRS_a07 := kernel/proc tests/host/a07 user/apps/init user/apps/ring3test \
	$(wildcard arch/x86_64/syscall*.c arch/x86_64/syscall*.h) \
	include/velum/proc.h include/velum/ksyscall.h
HT_a07 := $(sort $(wildcard tests/host/a07/test_*.c))
HSRC_a07 := kernel/proc/syscall_table.c kernel/proc/elf_hdr.c \
	kernel/proc/elf_seg.c kernel/proc/elf_final.c kernel/proc/elf_dyn.c \
	kernel/proc/elf_util.c kernel/proc/ustack.c kernel/proc/ustack_aux.c \
	kernel/proc/args.c kernel/proc/boot_args.c kernel/proc/ratelimit.c \
	arch/x86_64/syscall_check.c \
	user/apps/init/init_policy.c lib/libk/str.c lib/libk/str2.c \
	lib/libk/memcmp.c
HINC_a07 := -Ikernel/proc -Iarch/x86_64 -Iuser/apps/init
UAPPS += init ring3test
UAPP_init_LOT := a07
UAPP_init_SRC := $(sort $(wildcard user/apps/init/*.c))
UAPP_init_LIBS := velum
UAPP_ring3test_LOT := a07
UAPP_ring3test_SRC := $(sort $(wildcard user/apps/ring3test/*.c)) \
	user/apps/ring3test/r3_fault.S
UAPP_ring3test_LIBS := velum
