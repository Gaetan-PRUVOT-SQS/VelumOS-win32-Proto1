LOTS += a10
KSRC_a10 := $(sort $(wildcard drivers/input/*.c)) \
	$(sort $(wildcard kernel/input/*.c))
DIRS_a10 := drivers/input kernel/input tests/host/a10
HT_a10 := $(sort $(wildcard tests/host/a10/test_*.c))
HSRC_a10 := $(filter-out drivers/input/ps2_hw.c,$(KSRC_a10)) \
	lib/libk/fmt.c lib/libk/fmt_num.c lib/libk/fmt_out.c \
	lib/libk/fmt_parse.c lib/libk/fmt_str.c lib/libk/str.c lib/libk/str2.c \
	lib/libk/memcmp.c
HINC_a10 := -Idrivers/input -Ikernel/input
