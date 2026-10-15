LOTS += d00
KSRC_d00 :=
DIRS_d00 := tests/host/d00
D00_FIX := $(B)/d00-fixture
D00_TOOLS := tools/dexasm.py tools/mkapk.py $(wildcard tools/dexasm_lib/*.py)
D00_ESSAIS := $(sort $(wildcard user/apk/essais/*.dasm))
D00_BONJOUR := $(sort $(wildcard user/apk/bonjour/*.dasm))
D00_FILES := $(D00_FIX)/essais.dex $(D00_FIX)/bonjour.dex $(D00_FIX)/bonjour.apk \
	$(D00_FIX)/gros.apk
HT_d00 := $(sort $(wildcard tests/host/d00/test_*.c))
HSRC_d00 := $(sort $(wildcard lib/zip/*.c lib/axml/*.c lib/dex/*.c \
	lib/dexcode/*.c lib/dvm/*.c lib/droid/*.c lib/apk/*.c lib/rsa/*.c \
	lib/crypto/*.c)) lib/libk/str.c lib/libk/str2.c lib/libk/memcmp.c
HINC_d00 := -Itests/host/d00 -DD00_FIX='"$(D00_FIX)"'

$(D00_FIX)/essais.dex: $(D00_ESSAIS) $(D00_TOOLS)
	@mkdir -p $(dir $@)
	python3 tools/dexasm.py $(D00_ESSAIS) --sortie $@

$(D00_FIX)/bonjour.dex: $(D00_BONJOUR) $(D00_TOOLS)
	@mkdir -p $(dir $@)
	python3 tools/dexasm.py $(D00_BONJOUR) --sortie $@

$(D00_FIX)/bonjour.apk: $(D00_FIX)/bonjour.dex user/apk/bonjour/bonjour.toml \
	$(D00_TOOLS)
	python3 tools/mkapk.py user/apk/bonjour/bonjour.toml --dex $< \
		--sortie $@ --cles $(D00_FIX)/cles

$(D00_FIX)/gros.apk: $(D00_FIX)/bonjour.dex tests/host/d00/gen_gros.py \
	user/apk/bonjour/bonjour.toml $(D00_TOOLS)
	python3 tests/host/d00/gen_gros.py --sortie $(D00_FIX)/gros.toml
	python3 tools/mkapk.py $(D00_FIX)/gros.toml --dex $< \
		--sortie $@ --cles $(D00_FIX)/cles

host-d00: $(D00_FILES)

DIRS_d00 += user/apps/apkessai
UAPPS += apkessai
UAPP_apkessai_LOT := d00
UAPP_apkessai_SRC := $(sort $(wildcard user/apps/apkessai/*.c))
UAPP_apkessai_LIBS := velum

D00_KEYS := $(B)/apk-cles
D00_APP := $(O)/root/system/apps/com.velum.bonjour.apk

$(D00_APP): $(D00_BONJOUR) user/apk/bonjour/bonjour.toml $(D00_TOOLS)
	@mkdir -p $(dir $@) $(D00_KEYS)
	python3 tools/dexasm.py $(D00_BONJOUR) --sortie $(D00_KEYS)/bonjour.dex
	python3 tools/mkapk.py user/apk/bonjour/bonjour.toml \
		--dex $(D00_KEYS)/bonjour.dex --sortie $@ --cles $(D00_KEYS)

ROOTFS_FILES += $(D00_APP)
