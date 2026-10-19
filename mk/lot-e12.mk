LOTS += e12
KSRC_e12 :=
DIRS_e12 := lib/settings user/apps/reglages tests/host/e12 include/velum/settings.h
HT_e12 := $(sort $(wildcard tests/host/e12/test_*.c))
HSRC_e12 := $(sort $(wildcard lib/settings/*.c)) \
	$(A18_OWN) $(A18_LIBS) $(sort $(wildcard tests/host/a18/help_*.c)) \
	$(sort $(wildcard tests/host/a18/fake_*.c)) \
	lib/libk/str.c lib/libk/str2.c lib/libk/memcmp.c
HINC_e12 := -Ilib/settings -Iuser/apps/winsrv -Ilib/wm -Itests/host/e12 \
	-Itests/host/a18 -Ilib/gfx -Ilib/font -Ilib/luna -iquote user/include
ULIBS += settings
ULIB_settings_LOT := e12
ULIB_settings_SRC := $(sort $(wildcard lib/settings/*.c))
UAPPS += reglages
UAPP_reglages_LOT := e12
UAPP_reglages_SRC := $(sort $(wildcard user/apps/reglages/*.c))
UAPP_reglages_LIBS := a20kit ctl wm settings luna font gfx velum
