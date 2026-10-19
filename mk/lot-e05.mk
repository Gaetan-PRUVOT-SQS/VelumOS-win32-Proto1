LOTS += e05
DIRS_e05 := tests/host/e05
HT_e05 := $(sort $(wildcard tests/host/e05/test_*.c))
HSRC_e05 := fs/sys_io.c fs/sys_util.c lib/libk/str.c lib/libk/str2.c \
	lib/libk/memcmp.c
HINC_e05 := -Ifs
LOTS += e05t
DIRS_e05t := tests/host/e05t
HT_e05t := $(sort $(wildcard tests/host/e05t/test_*.c))
HSRC_e05t := kernel/proc/sys_thread.c lib/libk/str.c lib/libk/str2.c \
	lib/libk/memcmp.c
HINC_e05t := -Ikernel/proc
