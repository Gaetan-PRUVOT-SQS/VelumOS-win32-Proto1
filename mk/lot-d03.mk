LOTS += d03
KSRC_d03 :=
DIRS_d03 := lib/dex tests/host/d03 include/velum/apk/dex.h
HT_d03 := $(sort $(wildcard tests/host/d03/test_*.c))
HSRC_d03 := $(sort $(wildcard lib/dex/*.c)) lib/libk/str.c
D03_FIXTURE := $(B)/d03-fixture/classes.dex
HINC_d03 := -Ilib/dex -Itests/host/d03 -DD03_FIXTURE='"$(D03_FIXTURE)"'
ULIBS += dex
ULIB_dex_LOT := d03
ULIB_dex_SRC := $(sort $(wildcard lib/dex/*.c))

$(D03_FIXTURE): tests/host/d03/gen_dex.py
	@mkdir -p $(dir $@)
	python3 tests/host/d03/gen_dex.py --sortie $@

host-d03: $(D03_FIXTURE)
