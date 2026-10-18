LOTS += e10
KSRC_e10 := kernel/mm/meminfo.c kernel/mm/meminfo_build.c
DIRS_e10 := kernel/mm/meminfo.h kernel/mm/meminfo.c kernel/mm/meminfo_build.c \
	kernel/mm/heap_size.c include/velum/abi/abi_meminfo.h tests/host/e10
HT_e10 := $(sort $(wildcard tests/host/e10/test_*.c))
HSRC_e10 := kernel/mm/meminfo_build.c kernel/mm/heap_size.c \
	lib/libk/str.c lib/libk/str2.c lib/libk/memcmp.c
HINC_e10 := -Ikernel/mm
