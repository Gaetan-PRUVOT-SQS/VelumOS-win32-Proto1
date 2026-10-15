LOTS += skel
KSRC_skel := kernel/bootinfo.c kernel/buildid.c kernel/initcalls.c \
	kernel/klog.c kernel/klog_cfg.c kernel/klog_level.c kernel/main.c \
	kernel/panic.c kernel/selftests.c kernel/stackguard.c kernel/weaks.c \
	$(sort $(wildcard lib/libk/*.c)) lib/libk/mem.S \
	arch/x86_64/boot.c arch/x86_64/boot_memmap.c arch/x86_64/boot_misc.c \
	arch/x86_64/serial.c arch/x86_64/limine/requests.c \
	arch/x86_64/entry.S arch/x86_64/irqflags.S
DIRS_skel := lib/libk include/velum
HT_skel := $(sort $(wildcard tests/host/skel/test_*.c))
HSRC_skel := lib/libk/fmt.c lib/libk/fmt_num.c lib/libk/fmt_out.c \
	lib/libk/fmt_parse.c lib/libk/fmt_str.c lib/libk/str.c lib/libk/str2.c \
	lib/libk/memcmp.c
