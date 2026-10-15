LOTS += d07
KSRC_d07 :=
DIRS_d07 := lib/apk tests/host/d07 include/velum/apk/apk.h
D07_FIX := $(B)/d07-fixture
D07_GEN := tests/host/d07/gen_apk.py
D07_DASM := $(sort $(wildcard user/apk/bonjour/*.dasm))
D07_DEPS := tools/dexasm.py tools/mkapk.py $(wildcard tools/dexasm_lib/*.py) \
	$(D07_GEN) $(D07_DASM) user/apk/bonjour/bonjour.toml \
	tests/host/d07/relatif.toml
D07_MK := python3 tools/mkapk.py user/apk/bonjour/bonjour.toml \
	--dex $(D07_FIX)/bonjour.dex --cles $(D07_FIX)/cles
HT_d07 := $(sort $(wildcard tests/host/d07/test_*.c))
HSRC_d07 := $(sort $(wildcard lib/apk/*.c lib/zip/*.c lib/axml/*.c \
	lib/rsa/*.c lib/crypto/*.c)) lib/libk/str.c lib/libk/str2.c \
	lib/libk/memcmp.c
HINC_d07 := -Ilib/apk -Itests/host/d07 -DD07_FIX='"$(D07_FIX)"' \
	-Dmalloc=d07_malloc
ULIBS += apk
ULIB_apk_LOT := d07
ULIB_apk_SRC := $(sort $(wildcard lib/apk/*.c))

$(D07_FIX)/stamp: $(D07_DEPS)
	@mkdir -p $(D07_FIX)
	python3 tools/dexasm.py $(D07_DASM) --sortie $(D07_FIX)/bonjour.dex
	$(D07_MK) --sortie $(D07_FIX)/ok.apk
	$(D07_MK) --sortie $(D07_FIX)/nosig.apk --sans-signature
	for z in entree repertoire fin bloc; do \
		r=$$(python3 $(D07_GEN) --rang $$z $(D07_FIX)/ok.apk) || exit 1; \
		$(D07_MK) --sortie $(D07_FIX)/alt_$$z.apk --alterer $$r || exit 1; \
	done
	python3 tools/mkapk.py tests/host/d07/relatif.toml \
		--dex $(D07_FIX)/bonjour.dex --cles $(D07_FIX)/cles \
		--sortie $(D07_FIX)/relatif.apk
	python3 $(D07_GEN) $(D07_FIX)
	@touch $@

host-d07: $(D07_FIX)/stamp
