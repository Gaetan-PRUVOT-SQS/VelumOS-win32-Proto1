LOTS += a18
KSRC_a18 :=
DIRS_a18 := user/apps/winsrv lib/wm tests/host/a18 include/velum/wm.h
HT_a18 := $(sort $(wildcard tests/host/a18/test_*.c))
A18_SYS := user/apps/winsrv/main.c $(wildcard user/apps/winsrv/sys_*.c) \
	$(wildcard lib/wm/wmsys*.c)
A18_OWN := $(filter-out $(A18_SYS),$(sort $(wildcard user/apps/winsrv/*.c) \
	$(wildcard lib/wm/*.c)))
A18_LUNA ?= real
A18_LIBS := $(sort $(wildcard lib/gfx/*.c))
ifeq ($(A18_LUNA),real)
A18_LIBS += $(sort $(wildcard lib/font/*.c) $(wildcard lib/font/gen/*.c) \
	$(wildcard lib/luna/*.c))
else
A18_LIBS += $(sort $(wildcard tests/host/a18/stub_luna*.c))
endif
HSRC_a18 := $(A18_OWN) $(A18_LIBS) $(sort $(wildcard tests/host/a18/help_*.c)) \
	lib/libk/str.c lib/libk/str2.c lib/libk/memcmp.c
HINC_a18 := -Iuser/apps/winsrv -Ilib/wm -Itests/host/a18 -Ilib/gfx \
	-Ilib/font -Ilib/luna
ULIBS += wm
ULIB_wm_LOT := a18
ULIB_wm_SRC := $(sort $(wildcard lib/wm/*.c))
UAPPS += winsrv
UAPP_winsrv_LOT := a18
UAPP_winsrv_SRC := $(sort $(wildcard user/apps/winsrv/*.c))
UAPP_winsrv_LIBS := luna font gfx velum
DIRS_a18 += user/apps/wstress
UAPPS += wstress
UAPP_wstress_LOT := a18
UAPP_wstress_SRC := $(sort $(wildcard user/apps/wstress/*.c))
UAPP_wstress_LIBS := wm gfx velum
