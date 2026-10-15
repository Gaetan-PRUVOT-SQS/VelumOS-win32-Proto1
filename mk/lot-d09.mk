LOTS += d09
KSRC_d09 :=
DIRS_d09 := $(sort $(wildcard lib/dvm/in_*.c)) lib/dvm/in_int.h tests/host/d09
D09_FIX := $(B)/d09-fixture
D09_TOOLS := tools/dexasm.py $(wildcard tools/dexasm_lib/*.py)
D09_DASM := $(sort $(wildcard tests/host/d09/*.dasm))
D09_ESSAIS := $(sort $(wildcard user/apk/essais/*.dasm))
D09_FILES := $(D09_FIX)/d09.dex $(D09_FIX)/essais.dex
HT_d09 := $(sort $(wildcard tests/host/d09/test_*.c))
HSRC_d09 := $(sort $(wildcard lib/dvm/in_*.c lib/dex/*.c lib/dexcode/*.c)) \
	lib/libk/str.c
HINC_d09 := -Itests/host/d09 -Ilib/dvm -DD09_FIX='"$(D09_FIX)"' -lm
ULIBS += dvmin
ULIB_dvmin_LOT := d09
ULIB_dvmin_SRC := $(sort $(wildcard lib/dvm/in_*.c))

$(D09_FIX)/d09.dex: $(D09_DASM) $(D09_TOOLS)
	@mkdir -p $(dir $@)
	python3 tools/dexasm.py $(D09_DASM) --sortie $@

$(D09_FIX)/essais.dex: $(D09_ESSAIS) $(D09_TOOLS)
	@mkdir -p $(dir $@)
	python3 tools/dexasm.py $(D09_ESSAIS) --sortie $@

host-d09: $(D09_FILES)
