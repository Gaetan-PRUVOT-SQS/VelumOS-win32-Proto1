FUZZ_B ?= build/fuzz
FUZZ_CC_REPLAY ?= gcc
FUZZ_CC_LIB ?= clang
FUZZ ?=
TIME ?= 60
FUZZ_RSS_MB ?= 2048
FUZZ_UNIT_TIMEOUT ?= 10

FUZZ_TARGETS := zip inflate elf cpio dex axml fat acpi

FUZZ_ZLIB_A := $(firstword $(wildcard /usr/lib/x86_64-linux-gnu/libz.a \
	/usr/lib64/libz.a /usr/lib/libz.a))
FUZZ_ZLIB := $(and $(wildcard /usr/include/zlib.h),$(FUZZ_ZLIB_A))
FUZZ_ZFLAGS := $(if $(FUZZ_ZLIB),-DFUZZ_WITH_ZLIB -idirafter /usr/include)
FUZZ_ZLIBS := $(FUZZ_ZLIB)
FUZZ_LIBS_inflate := $(FUZZ_ZLIBS)

FUZZ_LIBK := lib/libk/str.c lib/libk/str2.c lib/libk/memcmp.c
FUZZ_DEFS := -DVELUM_HOST -DVELUM_FUZZ \
	$(foreach f,strlen strnlen strcmp strncmp strchr strrchr strstr strlcpy \
	strlcat memcmp memchr,-D$(f)=vk_$(f))
FUZZ_CFLAGS := $(FUZZ_ZFLAGS) -std=gnu11 -O1 -g -fno-omit-frame-pointer \
	-Wall -Wextra -Iinclude -Ilib/libk -Itests/fuzz $(FUZZ_DEFS)
FUZZ_SAN := -fsanitize=address,undefined -fno-sanitize-recover=undefined
FUZZ_COMMON := tests/fuzz/fuzz_core.c
FUZZ_TMP := $(abspath $(FUZZ_B))/tmp

FUZZ_SRC_zip := $(sort $(wildcard lib/zip/*.c)) $(FUZZ_LIBK)
FUZZ_INC_zip := -Ilib/zip
FUZZ_MAXLEN_zip := 65536

FUZZ_SRC_inflate := $(FUZZ_SRC_zip)
FUZZ_INC_inflate := $(FUZZ_INC_zip)
FUZZ_MAXLEN_inflate := 16384

FUZZ_SRC_elf := kernel/proc/elf_hdr.c kernel/proc/elf_seg.c \
	kernel/proc/elf_final.c kernel/proc/elf_dyn.c kernel/proc/elf_util.c \
	$(FUZZ_LIBK)
FUZZ_INC_elf := -Ikernel/proc
FUZZ_MAXLEN_elf := 65536

FUZZ_SRC_cpio := fs/cpio_index.c fs/cpio_parse.c fs/cpio_name.c \
	fs/cpio_sort.c fs/initrd_lookup.c fs/initrd_dir.c fs/vpath_utf8.c \
	$(FUZZ_LIBK)
FUZZ_INC_cpio := -Ifs
FUZZ_MAXLEN_cpio := 65536

FUZZ_SRC_fat := $(sort $(wildcard fs/fat_*.c)) fs/vname.c \
	$(sort $(wildcard fs/vpath*.c)) $(FUZZ_LIBK)
FUZZ_INC_fat := -Ifs
FUZZ_MAXLEN_fat := 21504

FUZZ_SRC_acpi := arch/x86_64/acpi/acpi_util.c arch/x86_64/acpi/acpi_rsdp.c \
	arch/x86_64/acpi/acpi_sdt.c arch/x86_64/acpi/acpi_madt.c \
	arch/x86_64/acpi/acpi_madt2.c arch/x86_64/acpi/acpi_fadt.c \
	arch/x86_64/acpi/acpi_fadt2.c arch/x86_64/acpi/acpi_s5.c \
	arch/x86_64/acpi/acpi_tables.c $(FUZZ_LIBK)
FUZZ_INC_acpi := -Iarch/x86_64/acpi
FUZZ_MAXLEN_acpi := 16384

FUZZ_SRC_dex := $(sort $(wildcard lib/dex/*.c)) \
	$(sort $(wildcard lib/dexcode/*.c)) tests/host/d03/fake_walk.c \
	$(FUZZ_LIBK)
FUZZ_INC_dex := -Ilib/dex -Ilib/dexcode -Itests/host/d03
FUZZ_MAXLEN_dex := 65536

FUZZ_SRC_axml := $(sort $(wildcard lib/axml/*.c)) tests/host/d02/fake_walk.c \
	$(FUZZ_LIBK)
FUZZ_INC_axml := -Ilib/axml -Itests/host/d02
FUZZ_MAXLEN_axml := 65536

define fuzz_rules
$$(FUZZ_B)/replay/fuzz_$(1): tests/fuzz/fuzz_$(1).c tests/fuzz/replay_main.c \
	$$(FUZZ_COMMON) $$(FUZZ_SRC_$(1)) $$(wildcard tests/fuzz/*.h) mk/qa-fuzz.mk
	@mkdir -p $$(dir $$@) $$(FUZZ_TMP)
	TMPDIR=$$(FUZZ_TMP) $$(FUZZ_CC_REPLAY) $$(FUZZ_CFLAGS) $$(FUZZ_SAN) -Werror $$(FUZZ_INC_$(1)) \
		tests/fuzz/fuzz_$(1).c tests/fuzz/replay_main.c $$(FUZZ_COMMON) \
		$$(FUZZ_SRC_$(1)) $$(FUZZ_LIBS_$(1)) -o $$@

$$(FUZZ_B)/bin/fuzz_$(1): tests/fuzz/fuzz_$(1).c $$(FUZZ_COMMON) \
	$$(FUZZ_SRC_$(1)) $$(wildcard tests/fuzz/*.h) mk/qa-fuzz.mk
	@mkdir -p $$(dir $$@) $$(FUZZ_TMP)
	TMPDIR=$$(FUZZ_TMP) $$(FUZZ_CC_LIB) $$(FUZZ_CFLAGS) -fsanitize=fuzzer,address,undefined \
		-fno-sanitize-recover=undefined $$(FUZZ_INC_$(1)) \
		tests/fuzz/fuzz_$(1).c $$(FUZZ_COMMON) $$(FUZZ_SRC_$(1)) $$(FUZZ_LIBS_$(1)) -o $$@
endef

$(foreach t,$(FUZZ_TARGETS),$(eval $(call fuzz_rules,$(t))))

FUZZ_REPLAY_BINS := $(foreach t,$(FUZZ_TARGETS),$(FUZZ_B)/replay/fuzz_$(t))

.PHONY: fuzz-replay fuzz fuzz-corpus-min fuzz-cov fuzz-list fuzz-build \
	fuzz-check-target

fuzz-replay: $(FUZZ_REPLAY_BINS)
	@set -e; \
	for t in $(FUZZ_TARGETS); do \
		printf 'fuzz-replay %s : ' "$$t"; \
		$(FUZZ_B)/replay/fuzz_$$t tests/fuzz/corpus/$$t || exit 1; \
	done; \
	echo 'fuzz-replay : OK'

fuzz-check-target:
	@case " $(FUZZ_TARGETS) " in *" $(FUZZ) "*) ;; \
		*) echo 'usage : make fuzz FUZZ=<cible> [TIME=60] ; cibles : $(FUZZ_TARGETS)' >&2; exit 2;; esac

fuzz-list: fuzz-check-target
	@echo '$(FUZZ_SRC_$(FUZZ))'

fuzz-build: fuzz-check-target
	@$(MAKE) --no-print-directory $(FUZZ_B)/bin/fuzz_$(FUZZ)

fuzz: fuzz-check-target
	@$(MAKE) --no-print-directory $(FUZZ_B)/bin/fuzz_$(FUZZ)
	@mkdir -p $(FUZZ_B)/work/$(FUZZ) $(FUZZ_B)/crash tests/fuzz/corpus/$(FUZZ)
	$(FUZZ_B)/bin/fuzz_$(FUZZ) -max_total_time=$(TIME) \
		-max_len=$(FUZZ_MAXLEN_$(FUZZ)) -rss_limit_mb=$(FUZZ_RSS_MB) \
		-timeout=$(FUZZ_UNIT_TIMEOUT) -print_final_stats=1 -seed=1 \
		$(if $(wildcard tests/fuzz/$(FUZZ).dict),-dict=tests/fuzz/$(FUZZ).dict) \
		-artifact_prefix=$(FUZZ_B)/crash/$(FUZZ)- \
		$(FUZZ_B)/work/$(FUZZ) tests/fuzz/corpus/$(FUZZ)

fuzz-corpus-min: fuzz-check-target
	@$(MAKE) --no-print-directory $(FUZZ_B)/bin/fuzz_$(FUZZ)
	@rm -rf $(FUZZ_B)/min/$(FUZZ)
	@mkdir -p $(FUZZ_B)/min/$(FUZZ) $(FUZZ_B)/work/$(FUZZ)
	$(FUZZ_B)/bin/fuzz_$(FUZZ) -merge=1 -max_len=$(FUZZ_MAXLEN_$(FUZZ)) \
		$(FUZZ_B)/min/$(FUZZ) tests/fuzz/corpus/$(FUZZ) $(FUZZ_B)/work/$(FUZZ)
	@echo 'corpus minimal : $(FUZZ_B)/min/$(FUZZ) (copier vers tests/fuzz/corpus/$(FUZZ) pour l adopter)'

fuzz-cov: fuzz-check-target
	@CC='$(FUZZ_CC_REPLAY)' CFLAGS='$(FUZZ_CFLAGS) -O0' INCS='$(FUZZ_INC_$(FUZZ))' \
		LIBS='$(FUZZ_LIBS_$(FUZZ))' sh tests/fuzz/cov.sh $(FUZZ) \
		$(FUZZ_B)/cov/$(FUZZ) '$(FUZZ_SRC_$(FUZZ))' \
		tests/fuzz/corpus/$(FUZZ) $(wildcard $(FUZZ_B)/work/$(FUZZ))
