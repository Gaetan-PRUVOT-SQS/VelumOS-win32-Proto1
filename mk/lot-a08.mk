LOTS += a08
KSRC_a08 := $(sort $(wildcard kernel/obj/*.c))
DIRS_a08 := kernel/obj tests/host/a08 include/velum
HT_a08 := $(sort $(wildcard tests/host/a08/test_*.c))
HSRC_a08 := $(filter-out kernel/obj/selftest%,$(sort $(wildcard kernel/obj/*.c))) \
	lib/libk/str.c lib/libk/memcmp.c
HINC_a08 := -Ikernel/obj
