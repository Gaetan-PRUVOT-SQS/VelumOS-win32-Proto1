LOTS += a20
KSRC_a20 :=
A20_PURE_COMMON := $(addprefix user/apps/common/,tbuf.c timefmt.c utf8.c layout.c)
A20_PURE_LOGON := $(addprefix user/apps/logon/,acc_line.c acc_parse.c acc_file.c \
	acc_verify.c limiter.c auth.c logon_flow.c logon_flow2.c logon_layout.c logon_layout_top.c logon_text.c)
A20_PURE_SHELL := $(addprefix user/apps/shell/,startmenu.c sm_items.c sm_nav.c \
	sm_input.c sm_layout.c tasklist.c tasklist_apply.c taskbar.c desktop.c \
	dblclick.c runcmd.c sm_progs.c sm_progs_line.c)
DIRS_a20 := user/apps/common user/apps/logon user/apps/shell user/apps/hello \
	tests/host/a20 tests/host/a20/render
HT_a20 := $(sort $(wildcard tests/host/a20/test_*.c))
HSRC_a20 := $(A20_PURE_COMMON) $(A20_PURE_LOGON) $(A20_PURE_SHELL) \
	$(sort $(wildcard lib/crypto/*.c)) lib/libk/str.c lib/libk/str2.c \
	lib/libk/memcmp.c tests/host/a20/a20_srgb.c
A20_FIXTURE := $(B)/a20-fixture/users
A20_FIXTURE_PW := $(B)/a20-fixture/users-mdp
HINC_a20 := -Iuser/apps/common -Iuser/apps/logon -Iuser/apps/shell \
	-Itests/host/a20 -DA20_FIXTURE='"$(A20_FIXTURE)"' \
	-DA20_FIXTURE_PW='"$(A20_FIXTURE_PW)"' \
	-DA20_RENDER='"$(B)/a20-render"'

$(A20_FIXTURE): tools/mkuser.py
	@mkdir -p $(dir $@)
	python3 tools/mkuser.py --sortie $@ --compte Utilisateur \
		--sel-hex 000102030405060708090a0b0c0d0e0f

$(A20_FIXTURE_PW): tools/mkuser.py
	@mkdir -p $(dir $@)
	printf '%s\n' 'Passe 123' | python3 tools/mkuser.py --sortie $@ \
		--compte Alice --iterations 12000 --sel-hex a1b2c3d4e5f60718 \
		--mot-de-passe-stdin

.PHONY: a20-mkuser
a20-mkuser: $(A20_FIXTURE) $(A20_FIXTURE_PW)
	A20_FIXTURE=$(A20_FIXTURE) python3 tests/host/a20/test_mkuser.py

A20_RDIR := tests/host/a20/render
A20_RTESTS := $(sort $(wildcard $(A20_RDIR)/test_*.c))
A20_RFAKES := $(sort $(filter-out $(A20_RTESTS),$(wildcard $(A20_RDIR)/*.c)))
A20_RAPP := $(filter-out user/apps/common/os_%.c user/apps/common/apkglue%.c,$(sort $(wildcard \
	user/apps/common/*.c))) $(filter-out %_start.c,$(sort $(wildcard \
	user/apps/shell/*.c user/apps/logon/*.c user/apps/hello/*.c)))
A20_RLIBS := $(sort $(wildcard lib/gfx/*.c lib/font/*.c lib/font/gen/*.c \
	lib/luna/*.c lib/ctl/*.c lib/crypto/*.c)) lib/libk/str.c lib/libk/str2.c \
	lib/libk/memcmp.c tests/host/harness.c tests/host/harness_eq.c
A20_ROBJ := $(patsubst %.c,$(B)/host/a20r/obj/%.o,$(A20_RFAKES) $(A20_RAPP) \
	$(A20_RLIBS))
A20_RFLAGS = $(HOSTFLAGS) $(HINC_a20) -I$(A20_RDIR) -Ilib/gfx -Ilib/font \
	-Ilib/luna -Ilib/ctl

$(B)/host/a20r/obj/%.o: %.c
	@mkdir -p $(dir $@)
	$(HOSTCC) $(A20_RFLAGS) -MMD -MP -c $< -o $@

$(B)/host/a20r/%: $(A20_RDIR)/%.c $(A20_ROBJ)
	$(HOSTCC) $(A20_RFLAGS) $< $(A20_ROBJ) -o $@

-include $(A20_ROBJ:.o=.d)

.PHONY: a20-render
a20-render: $(patsubst $(A20_RDIR)/%.c,$(B)/host/a20r/%,$(A20_RTESTS))
	@python3 tools/suite_hote.py $^

.PHONY: a20-typo
a20-typo:
	python3 tests/host/a20/test_typographie.py

host-a20: a20-mkuser a20-typo a20-render $(A20_FIXTURE) $(A20_FIXTURE_PW)

ULIBS += a20kit
ULIB_a20kit_LOT := a20
ULIB_a20kit_SRC := $(sort $(wildcard user/apps/common/*.c))
A20_LIBS := a20kit ctl wm luna font gfx crypto velum
UAPPS += logon shell hello
UAPP_logon_LOT := a20
UAPP_logon_SRC := $(sort $(wildcard user/apps/logon/*.c))
UAPP_logon_LIBS := $(A20_LIBS)
UAPP_shell_LOT := a20
UAPP_shell_SRC := $(sort $(wildcard user/apps/shell/*.c))
UAPP_shell_LIBS := $(A20_LIBS) settings
UAPP_hello_LOT := a20
UAPP_hello_SRC := $(sort $(wildcard user/apps/hello/*.c))
UAPP_hello_LIBS := $(A20_LIBS)

ifneq ($(filter a20,$(or $(ONLY),a20)),)
A20_ROOTFS_SRC := $(sort $(shell find rootfs -type f))
A20_ROOTFS_OUT := $(patsubst rootfs/%,$(O)/root/%,$(A20_ROOTFS_SRC))
A20_USERS := $(O)/root/system/etc/users
ROOTFS_FILES += $(A20_ROOTFS_OUT) $(A20_USERS)

$(A20_ROOTFS_OUT): $(O)/root/%: rootfs/%
	@mkdir -p $(dir $@)
	cp $< $@

$(A20_USERS): tools/mkuser.py
	@mkdir -p $(dir $@)
	python3 tools/mkuser.py --sortie $@ --compte Utilisateur
endif
