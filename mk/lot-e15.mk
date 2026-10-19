LOTS += e15
KSRC_e15 :=
E15_PURE := con_screen con_line con_args con_path con_core con_table \
	con_cmd_fs con_cmd_sys con_cmd_misc con_view
DIRS_e15 := user/apps/console tests/host/e15
HT_e15 := $(sort $(wildcard tests/host/e15/test_*.c))
HSRC_e15 := $(addprefix user/apps/console/,$(addsuffix .c,$(E15_PURE))) \
	user/apps/common/tbuf.c user/apps/common/utf8.c lib/libk/str.c \
	lib/libk/str2.c lib/libk/memcmp.c lib/font/font_table.c \
	lib/font/font_glyph.c lib/font/font_utf8.c $(sort $(wildcard lib/font/gen/*.c))
HINC_e15 := -Iuser/apps/console -Iuser/apps/common -Itests/host/e15 -Ilib/font
UAPPS += console
UAPP_console_LOT := e15
UAPP_console_SRC := $(sort $(wildcard user/apps/console/*.c))
UAPP_console_LIBS := a20kit ctl wm luna font gfx velum
