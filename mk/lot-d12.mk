LOTS += d12
KSRC_d12 :=
D12_GLUE := $(addprefix user/apps/common/,apkglue.c apkglue_fs.c \
	apkglue_fsio.c apkglue_insp.c)
DIRS_d12 := user/apps/apkrun user/apps/apkinst tests/host/d12 $(D12_GLUE) \
	user/apps/common/apkglue.h
HT_d12 := $(sort $(wildcard tests/host/d12/test_*.c))
D12_FIX := $(B)/d12-fixture
D12_DASM := $(sort $(wildcard user/apk/bonjour/*.dasm))
D12_MK := python3 tools/mkapk.py user/apk/bonjour/bonjour.toml \
	--dex $(D12_FIX)/bonjour.dex --cles $(D12_FIX)/cles
HSRC_d12 := $(D12_GLUE) user/apps/apkrun/apkrun_lay.c \
	$(sort $(wildcard lib/apk/*.c lib/zip/*.c lib/axml/*.c lib/rsa/*.c \
	lib/crypto/*.c)) lib/libk/str.c lib/libk/str2.c lib/libk/memcmp.c
HINC_d12 := -idirafter user/include -Iuser/apps/common -Iuser/apps/apkrun \
	-Itests/host/d12 -DD12_FIX='"$(D12_FIX)"'
UAPPS += apkrun apkinst
UAPP_apkrun_LOT := d12
UAPP_apkrun_SRC := $(sort $(wildcard user/apps/apkrun/*.c))
UAPP_apkrun_LIBS := a20kit ctl wm luna font gfx droid dvm dvmin dex dexcode \
	apk axml zip rsa crypto velum
UAPP_apkinst_LOT := d12
UAPP_apkinst_SRC := $(sort $(wildcard user/apps/apkinst/*.c))
UAPP_apkinst_LIBS := a20kit pm apk axml zip rsa crypto velum

$(D12_FIX)/stamp: tools/dexasm.py tools/mkapk.py \
		$(wildcard tools/dexasm_lib/*.py) $(D12_DASM) \
		user/apk/bonjour/bonjour.toml
	@mkdir -p $(D12_FIX)
	python3 tools/dexasm.py $(D12_DASM) --sortie $(D12_FIX)/bonjour.dex
	$(D12_MK) --sortie $(D12_FIX)/ok.apk
	$(D12_MK) --sortie $(D12_FIX)/nosig.apk --sans-signature
	@touch $@

host-d12: $(D12_FIX)/stamp
