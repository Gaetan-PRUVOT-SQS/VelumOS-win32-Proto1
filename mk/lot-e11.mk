LOTS += e11
KSRC_e11 :=
E11_PURE := $(addprefix user/apps/fichiers/,fichiers_path.c fichiers_text.c \
	fichiers_ops.c)
DIRS_e11 := user/apps/fichiers tests/host/e11
HT_e11 := $(sort $(wildcard tests/host/e11/test_*.c))
HSRC_e11 := $(E11_PURE) user/apps/fichiers/fichiers_vfs.c \
	user/apps/common/tbuf.c lib/libk/str.c lib/libk/str2.c
HINC_e11 := -Iuser/apps/common -Iuser/apps/fichiers -Itests/host/e11 \
	-iquote user/include

UAPPS += fichiers
UAPP_fichiers_LOT := e11
UAPP_fichiers_SRC := $(sort $(wildcard user/apps/fichiers/*.c))
UAPP_fichiers_LIBS := a20kit ctl wm luna font gfx velum
