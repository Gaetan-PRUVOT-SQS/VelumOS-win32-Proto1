LOTS += e04
KSRC_e04 :=
DIRS_e04 := lib/pm user/apps/apkrun tests/host/e04 tests/host/d12
HT_e04 := $(sort $(wildcard tests/host/e04/test_*.c)) \
	tests/host/d12/sec_registre.c
HSRC_e04 := $(filter-out lib/pm/pm_mem.c,$(sort $(wildcard lib/pm/*.c))) \
	user/apps/apkrun/apkrun_trust.c \
	$(addprefix tests/host/d10/,fake_fs.c fake_fs_ops.c fake_insp.c fake_util.c) \
	lib/libk/str.c lib/libk/str2.c lib/libk/memcmp.c
HINC_e04 := -Ilib/pm -Itests/host/d10 -Iuser/apps/apkrun
UAPP_apkrun_LIBS += pm

LOTS += e04a
KSRC_e04a :=
DIRS_e04a := lib/pm user/apps/apkrun tests/host/d12
HT_e04a := tests/host/d12/sec_lancement.c
HSRC_e04a := $(filter-out lib/pm/pm_mem.c,$(sort $(wildcard lib/pm/*.c))) \
	user/apps/apkrun/apkrun_load.c user/apps/apkrun/apkrun_trust.c \
	$(addprefix tests/host/d10/,fake_fs.c fake_fs_ops.c fake_insp.c fake_util.c) \
	lib/libk/str.c lib/libk/str2.c lib/libk/memcmp.c
HINC_e04a := -Ilib/pm -Itests/host/d10 -Iuser/apps/apkrun -Iuser/apps/common
