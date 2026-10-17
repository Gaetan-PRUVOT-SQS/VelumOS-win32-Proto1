ANA := build/analyse
ANA_PROFIL ?= release
ANA_NIVEAU ?= rapide
ANA_JOBS ?= 3
ifeq ($(ANA_NIVEAU),exhaustif)
ANA_GCC := fanalyzer-profond
ANA_CPPCHECK_NIVEAU := exhaustive
ANA_EXPLOSION := 200
ANA_ENODES := 64
else
ANA_GCC := fanalyzer
ANA_CPPCHECK_NIVEAU := normal
ANA_EXPLOSION := 20
ANA_ENODES := 8
endif
ANA_OUTILS ?= $(ANA_GCC) cppcheck-$(ANA_CPPCHECK_NIVEAU) clang-tidy
ANA_BASE ?= tools/analyse.baseline
ANA_CC ?= $(if $(shell command -v $(CC) 2>/dev/null),$(CC),$(HOSTCC))
ANA_OPT ?= -O1

ANA_EXCLUS := third_party/% lib/font/gen/% build/% tests/%
ANA_KSRC := $(sort $(filter-out $(ANA_EXCLUS),$(filter %.c,$(foreach l,$(LOTS),$(KSRC_$(l))))))
ANA_USRC := $(sort $(filter-out $(ANA_EXCLUS),$(filter %.c,$(USRC))))
ANA_TLOTS := $(foreach l,$(LOTS),$(if $(wildcard tests/host/$(l)/*.c),$(l)))
ANA_TROOT := tests/host/harness.c tests/host/harness_eq.c

ANA_ISYS := -isystem third_party/limine
ANA_KGEN := -std=gnu11 -ffreestanding -fno-pic -fno-pie -mcmodel=kernel \
	-mno-red-zone -mgeneral-regs-only -fno-strict-aliasing -fwrapv -fno-common \
	-fno-delete-null-pointer-checks
ANA_KDEF := -DVELUM_KERNEL $(filter -D%,$(KCFLAGS_$(ANA_PROFIL)))
ANA_KINC := -Iinclude $(ANA_ISYS) -Iarch/x86_64/limine
ANA_UGEN := -std=gnu11 -ffreestanding -fPIE -fno-strict-aliasing -fwrapv -fno-common
ANA_UDEF := -DVELUM_USER
ANA_UINC := -Iinclude -Iuser/include
ANA_TGEN := $(filter-out -fsanitize% -fno-sanitize% -Werror -g -O%,$(HOSTFLAGS)) -Wno-error

ANA_GCC_OPTS := $(ANA_OPT) -fanalyzer -Wanalyzer-too-complex -fdiagnostics-plain-output \
	--param=analyzer-bb-explosion-factor=$(ANA_EXPLOSION) \
	--param=analyzer-max-enodes-per-program-point=$(ANA_ENODES) -Wno-error
ANA_CPPCHECK_OPTS := --quiet --language=c --std=c11 --platform=unix64 \
	--enable=warning,style,performance,portability --check-level=$(ANA_CPPCHECK_NIVEAU) \
	--suppressions-list=tools/cppcheck.suppress \
	--template='{file}|{line}|{severity}|{id}|{message}'
ANA_TIDY_OPTS := --quiet --config-file=.clang-tidy --use-color=false

define ana_regles
ANA_RAW_$(1) := $$(foreach t,$$(ANA_OUTILS),$$(patsubst %,$(ANA)/raw/$$(t)/$(1)/%.txt,$(5)))

$(ANA)/raw/$(ANA_GCC)/$(1)/$(2)%.c.txt: $(2)%.c
	@mkdir -p $$(dir $$@)
	@printf '  FANALYZER %s\n' $$<
	$(Q)$(3) $(4) $$(ANA_GCC_OPTS) -MMD -MF $$(@:.txt=.d) -MT $$@ \
		-MT $(ANA)/raw/cppcheck-$$(ANA_CPPCHECK_NIVEAU)/$(1)/$(2)$$*.c.txt \
		-MT $(ANA)/raw/clang-tidy/$(1)/$(2)$$*.c.txt \
		-c $$< -o /dev/null > $$@ 2>&1 || { cat $$@; rm -f $$@; exit 1; }

$(ANA)/raw/cppcheck-$$(ANA_CPPCHECK_NIVEAU)/$(1)/$(2)%.c.txt: $(2)%.c tools/cppcheck.suppress
	@mkdir -p $$(dir $$@)
	@printf '  CPPCHECK  %s\n' $$<
	$(Q)cppcheck $$(ANA_CPPCHECK_OPTS) $$(filter -D% -I%,$$(subst -isystem ,-I,$(4))) \
		$$< 2> $$@

$(ANA)/raw/clang-tidy/$(1)/$(2)%.c.txt: $(2)%.c .clang-tidy
	@mkdir -p $$(dir $$@)
	@printf '  TIDY      %s\n' $$<
	$(Q)clang-tidy $$(ANA_TIDY_OPTS) $$< -- --target=x86_64-unknown-elf $(4) \
		> $$@ 2> $$@.err || true
endef

$(eval $(call ana_regles,k,,$(ANA_CC),$(ANA_KGEN) $(ANA_KDEF) $(ANA_KINC),$(ANA_KSRC)))
$(eval $(call ana_regles,u,,$(ANA_CC),$(ANA_UGEN) $(ANA_UDEF) $(ANA_UINC),$(ANA_USRC)))
$(eval $(call ana_regles,t,tests/host/,$(HOSTCC),$(ANA_TGEN) -Itests/host,$(ANA_TROOT)))
$(foreach l,$(ANA_TLOTS),$(eval $(call ana_regles,t$(l),tests/host/$(l)/,$(HOSTCC),\
	$(ANA_TGEN) $(filter-out -l% -Wl%,$(HINC_$(l))),\
	$(sort $(wildcard tests/host/$(l)/*.c)))))

ANA_RAW_CODE := $(ANA_RAW_k) $(ANA_RAW_u)
ANA_RAW_TESTS := $(ANA_RAW_t) $(foreach l,$(ANA_TLOTS),$(ANA_RAW_t$(l)))

.PHONY: analyse analyse-tests analyse-baseline analyse-tests-baseline analyse-outils \
	analyse-clean analyse-raw-code analyse-raw-tests

analyse-outils:
	@for t in cppcheck clang-tidy $(ANA_CC) $(HOSTCC); do \
		command -v $$t > /dev/null || { echo "outil absent : $$t" >&2; exit 1; }; \
	done

analyse-raw-code: $(ANA_RAW_CODE)
analyse-raw-tests: $(ANA_RAW_TESTS)

analyse: analyse-outils
	@$(MAKE) --no-print-directory -j$(ANA_JOBS) analyse-raw-code
	@python3 tools/analyse.py --portee code --outils $(ANA_OUTILS) --base $(ANA_BASE) $(ANA_ARGS)

analyse-tests: analyse-outils
	@$(MAKE) --no-print-directory -j$(ANA_JOBS) analyse-raw-tests
	@python3 tools/analyse.py --portee tests --outils $(ANA_OUTILS) --base $(ANA_BASE) $(ANA_ARGS)

analyse-baseline:
	@$(MAKE) --no-print-directory analyse ANA_ARGS=--update-baseline

analyse-tests-baseline:
	@$(MAKE) --no-print-directory analyse-tests ANA_ARGS=--update-baseline

analyse-clean:
	rm -rf $(ANA)

-include $(patsubst %.txt,%.d,$(filter $(ANA)/raw/$(ANA_GCC)/%,$(ANA_RAW_CODE) $(ANA_RAW_TESTS)))
