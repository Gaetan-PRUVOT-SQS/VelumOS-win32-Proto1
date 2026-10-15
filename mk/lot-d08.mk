LOTS += d08
KSRC_d08 :=
DIRS_d08 := tests/host/d08
D08_FIX := $(B)/d08-fixture
D08_TOOLS := tools/dexasm.py $(wildcard tools/dexasm_lib/*.py)
D08_DASM := $(sort $(wildcard tests/host/d08/*.dasm))
HT_d08 := $(sort $(wildcard tests/host/d08/test_*.c))
HSRC_d08 := $(sort $(wildcard lib/dvm/rt_*.c lib/dex/*.c lib/dexcode/*.c)) \
	lib/libk/str.c
HINC_d08 := -Ilib/dvm -Itests/host/d08 -Wl,--wrap=calloc \
	-DD08_FIX='"$(D08_FIX)"'
ULIBS += dvm
ULIB_dvm_LOT := d08
ULIB_dvm_SRC := $(sort $(wildcard lib/dvm/rt_*.c))

$(D08_FIX)/essai.dex: $(D08_DASM) $(D08_TOOLS)
	@mkdir -p $(dir $@)
	python3 tools/dexasm.py $(D08_DASM) --sortie $@

host-d08: $(D08_FIX)/essai.dex
