LOTS += d01
KSRC_d01 :=
DIRS_d01 := lib/zip tests/host/d01 include/velum/apk/zip.h
HT_d01 := $(sort $(wildcard tests/host/d01/test_*.c))
HSRC_d01 := $(sort $(wildcard lib/zip/*.c)) lib/libk/str.c
D01_FIX := $(B)/d01-fixture
HINC_d01 := -Ilib/zip -Itests/host/d01 -DD01_FIX='"$(D01_FIX)"'

$(D01_FIX)/stamp: tests/host/d01/mkfix.py
	@mkdir -p $(dir $@)
	python3 tests/host/d01/mkfix.py $(D01_FIX)
	touch $@

host-d01: $(D01_FIX)/stamp

ULIBS += zip
ULIB_zip_LOT := d01
ULIB_zip_SRC := $(sort $(wildcard lib/zip/*.c))
