LOTS += d02
KSRC_d02 :=
DIRS_d02 := lib/axml tests/host/d02 include/velum/apk/respool.h \
	include/velum/apk/axml.h include/velum/apk/arsc.h
HT_d02 := $(sort $(wildcard tests/host/d02/test_*.c))
HSRC_d02 := $(sort $(wildcard lib/axml/*.c)) lib/libk/str.c
D02_FIXDIR := $(B)/d02-fixture
D02_FIX := $(D02_FIXDIR)/stamp
HINC_d02 := -Ilib/axml -Itests/host/d02 -DD02_FIXTURE='"$(D02_FIXDIR)"'

$(D02_FIX): tests/host/d02/gen_fixtures.py
	@mkdir -p $(D02_FIXDIR)
	python3 tests/host/d02/gen_fixtures.py $(D02_FIXDIR)
	@touch $@

host-d02: $(D02_FIX)

ULIBS += axml
ULIB_axml_LOT := d02
ULIB_axml_SRC := $(sort $(wildcard lib/axml/*.c))
