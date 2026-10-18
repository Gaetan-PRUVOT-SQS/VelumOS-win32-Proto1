LOTS += e19
KSRC_e19 := $(sort $(wildcard kernel/net/*.c drivers/net/*.c))
DIRS_e19 := kernel/net drivers/net include/velum/net.h \
	include/velum/abi/abi_net.h user/libvelum/sys_net.c tests/host/e19
HT_e19 := $(sort $(wildcard tests/host/e19/test_*.c))
HSRC_e19 := $(filter-out kernel/net/net_boot.c kernel/net/net_sys.c,\
	$(sort $(wildcard kernel/net/*.c))) lib/libk/str.c lib/libk/str2.c \
	lib/libk/memcmp.c
HINC_e19 := -Ikernel/net -Itests/host/e19
