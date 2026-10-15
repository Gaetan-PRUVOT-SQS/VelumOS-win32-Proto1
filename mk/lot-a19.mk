LOTS += a19
KSRC_a19 :=
DIRS_a19 := lib/ctl tests/host/a19 include/velum/ctl.h
HT_a19 := $(sort $(wildcard tests/host/a19/test_*.c))
HSRC_a19 := $(filter-out lib/ctl/ctl_mem.c,$(sort $(wildcard lib/ctl/*.c))) \
	lib/libk/str.c
HINC_a19 := -Ilib/ctl
ULIBS += ctl
ULIB_ctl_LOT := a19
ULIB_ctl_SRC := $(sort $(wildcard lib/ctl/*.c))
