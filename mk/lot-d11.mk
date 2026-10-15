LOTS += d11
KSRC_d11 :=
DIRS_d11 := lib/droid tests/host/d11
D11_FIX := $(B)/d11-fixture
D11_TOOLS := tools/dexasm.py $(wildcard tools/dexasm_lib/*.py)
D11_ESSAIS := $(sort $(wildcard user/apk/essais/*.dasm))
D11_BONJOUR := $(sort $(wildcard user/apk/bonjour/*.dasm))
D11_DASM := $(sort $(wildcard tests/host/d11/*.dasm))
D11_FILES := $(D11_FIX)/essais.dex $(D11_FIX)/bonjour.dex $(D11_FIX)/d11.dex
HT_d11 := $(sort $(wildcard tests/host/d11/test_*.c))
HSRC_d11 := $(sort $(wildcard lib/droid/*.c lib/dvm/rt_*.c lib/dvm/in_*.c \
	lib/dex/*.c lib/dexcode/*.c)) lib/libk/str.c
HINC_d11 := -Itests/host/d11 -Ilib/droid -DD11_FIX='"$(D11_FIX)"' -lm
ULIBS += droid
ULIB_droid_LOT := d11
ULIB_droid_SRC := $(sort $(wildcard lib/droid/*.c))

$(D11_FIX)/essais.dex: $(D11_ESSAIS) $(D11_TOOLS)
	@mkdir -p $(dir $@)
	python3 tools/dexasm.py $(D11_ESSAIS) --sortie $@

$(D11_FIX)/bonjour.dex: $(D11_BONJOUR) $(D11_TOOLS)
	@mkdir -p $(dir $@)
	python3 tools/dexasm.py $(D11_BONJOUR) --sortie $@

$(D11_FIX)/d11.dex: $(D11_DASM) $(D11_TOOLS)
	@mkdir -p $(dir $@)
	python3 tools/dexasm.py $(D11_DASM) --sortie $@

host-d11: $(D11_FILES)
