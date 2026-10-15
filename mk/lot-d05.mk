LOTS += d05
KSRC_d05 :=
DIRS_d05 := lib/rsa tests/host/d05 include/velum/apk/rsa.h
D05_FIX := $(B)/d05-fixture
D05_VEC := $(D05_FIX)/d05_vec.c
HT_d05 := $(sort $(wildcard tests/host/d05/test_*.c))
HSRC_d05 := $(sort $(wildcard lib/rsa/*.c)) $(sort $(wildcard lib/crypto/*.c)) \
	lib/libk/str.c lib/libk/str2.c lib/libk/memcmp.c $(D05_VEC)
HINC_d05 := -Ilib/rsa -Itests/host/d05
ULIBS += rsa
ULIB_rsa_LOT := d05
ULIB_rsa_SRC := $(sort $(wildcard lib/rsa/*.c))

$(D05_VEC): tests/host/d05/mkvec.py
	@mkdir -p $(D05_FIX)
	python3 tests/host/d05/mkvec.py --dir $(D05_FIX) --out $@
