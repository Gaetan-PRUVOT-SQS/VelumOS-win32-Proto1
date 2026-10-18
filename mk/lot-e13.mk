LOTS += e13
KSRC_e13 :=
DIRS_e13 := lib/notif user/apps/notif tests/host/e13 include/velum/notif.h
HT_e13 := $(sort $(wildcard tests/host/e13/test_*.c))
HSRC_e13 := lib/notif/notif_store.c lib/notif/notif_wire.c \
	lib/notif/notif_text.c lib/libk/str.c lib/libk/str2.c lib/libk/memcmp.c
HINC_e13 :=
UAPPS += notif
UAPP_notif_LOT := e13
UAPP_notif_SRC := $(sort $(wildcard user/apps/notif/*.c)) \
	lib/notif/notif_store.c lib/notif/notif_wire.c lib/notif/notif_text.c \
	lib/notif/notif_client.c
UAPP_notif_LIBS := a20kit ctl wm luna font gfx velum
UAPP_shell_SRC += lib/notif/notif_client.c lib/notif/notif_wire.c \
	lib/notif/notif_text.c
