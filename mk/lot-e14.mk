LOTS += e14
KSRC_e14 :=
DIRS_e14 := user/apps/shell tests/host/e14
HT_e14 := $(sort $(wildcard tests/host/e14/test_*.c))
HSRC_e14 := user/apps/shell/sh_launch_apk.c lib/libk/str.c lib/libk/str2.c lib/libk/memcmp.c
HINC_e14 := -Iuser/apps/shell
