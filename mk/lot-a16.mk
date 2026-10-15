LOTS += a16
KSRC_a16 := $(sort $(wildcard lib/font/*.c)) $(sort $(wildcard lib/font/gen/*.c))
DIRS_a16 := lib/font include/velum/font.h tests/host/a16
HT_a16 := $(sort $(wildcard tests/host/a16/test_*.c))
HSRC_a16 := $(KSRC_a16) lib/libk/str.c lib/libk/str2.c lib/libk/memcmp.c
HINC_a16 := -Ilib/font
ULIBS += font
ULIB_font_LOT := a16
ULIB_font_SRC := $(KSRC_a16)

A16_XCHECK := $(B)/host/a16/crosscheck_utf8

host-a16: export A16_OUT := $(B)/render
host-a16: | $(B)/render

$(B)/render $(B)/genfont:
	mkdir -p $@

.PHONY: genfont genfont-check fontsheets fontrender utf8-crosscheck

genfont:
	python3 tools/genfont.py

genfont-check: | $(B)/genfont
	python3 tools/genfont.py --out $(B)/genfont/a > $(B)/genfont/a.sha256
	python3 tools/genfont.py --out $(B)/genfont/b > $(B)/genfont/b.sha256
	cmp $(B)/genfont/a.sha256 $(B)/genfont/b.sha256
	python3 tools/genfont.py --check
	python3 -m unittest tests/host/a16/test_genfont.py

fontsheets:
	python3 tools/genfont.py --check --sheets $(B)/sheets

fontrender: $(B)/host/a16/test_render | $(B)/render
	A16_OUT=$(B)/render $(B)/host/a16/test_render
	python3 tools/genfont_view.py $(B)/render/*.ppm

$(A16_XCHECK): tests/host/a16/crosscheck_utf8.c lib/font/font_utf8.c
	@mkdir -p $(dir $@)
	$(HOSTCC) $(HOSTFLAGS) -Ilib/font $^ -o $@

utf8-crosscheck: $(A16_XCHECK)
	python3 tests/host/a16/crosscheck_utf8.py $(A16_XCHECK)

A16_INTEG := $(B)/host/a16/integ_gfx

$(A16_INTEG): tests/host/a16/integ_gfx.c tests/host/a16/fake_buf.c tests/host/a16/fake_font.c \
	tests/host/a16/fake_paint.c tests/host/harness.c tests/host/harness_eq.c $(KSRC_a16) \
	$(sort $(wildcard lib/gfx/*.c)) lib/libk/str.c lib/libk/str2.c lib/libk/memcmp.c
	@mkdir -p $(dir $@)
	$(HOSTCC) $(HOSTFLAGS) -Ilib/font -Ilib/gfx -Itests/host/a16 $^ -o $@

.PHONY: integ-a16
integ-a16: $(A16_INTEG)
	$(A16_INTEG)

.PHONY: a16-hostflags
a16-hostflags:
	@echo '$(HOSTFLAGS)'
