LOTS += a12
KSRC_a12 := $(sort $(wildcard fs/*.c))
DIRS_a12 := fs tests/host/a12 include/velum/vfs.h
HT_a12 := $(sort $(wildcard tests/host/a12/test_*.c))
HSRC_a12 := $(sort $(wildcard fs/*.c)) lib/libk/str.c lib/libk/str2.c \
	lib/libk/memcmp.c
HINC_a12 := -Ifs
