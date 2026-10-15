LOTS += d10
KSRC_d10 :=
DIRS_d10 := lib/pm tests/host/d10 include/velum/apk/pm.h
HT_d10 := $(sort $(wildcard tests/host/d10/test_*.c))
HSRC_d10 := $(filter-out lib/pm/pm_mem.c,$(sort $(wildcard lib/pm/*.c))) \
	lib/libk/str.c lib/libk/str2.c lib/libk/memcmp.c
HINC_d10 := -Ilib/pm -Itests/host/d10
ULIBS += pm
ULIB_pm_LOT := d10
ULIB_pm_SRC := $(sort $(wildcard lib/pm/*.c))
