LOTS += a02
KSRC_a02 := $(sort $(wildcard kernel/mm/pmm*.c))
DIRS_a02 := $(sort $(wildcard kernel/mm/pmm*.c kernel/mm/pmm*.h)) \
	include/velum/pmm.h tests/host/a02
HT_a02 := $(sort $(wildcard tests/host/a02/test_*.c))
HSRC_a02 := $(KSRC_a02) lib/libk/str.c lib/libk/str2.c lib/libk/memcmp.c
HINC_a02 := -Ikernel/mm -DVELUM_DEBUG

COVDIR_a02 := $(B)/cover/a02
COVOBJ_a02 := $(patsubst %.c,$(COVDIR_a02)/%.o,$(HSRC_a02))
COVBIN_a02 := $(patsubst tests/host/a02/%.c,$(COVDIR_a02)/bin/%,$(HT_a02))
COVFLAGS_a02 = $(filter-out -fsanitize=% -fno-sanitize-recover=%,$(HOSTFLAGS)) \
	-O0 --coverage -fcondition-coverage -fprofile-abs-path

$(COVDIR_a02)/%.o: %.c
	@mkdir -p $(dir $@)
	$(HOSTCC) $(COVFLAGS_a02) $(HINC_a02) -c $< -o $@

$(COVDIR_a02)/bin/%: tests/host/a02/%.c $(COVOBJ_a02) \
		$(wildcard tests/host/a02/fake_*.c) $(wildcard tests/host/a02/*.h)
	@mkdir -p $(dir $@)
	$(HOSTCC) $(COVFLAGS_a02) $(HINC_a02) $< tests/host/harness.c \
		tests/host/harness_eq.c $(wildcard tests/host/a02/fake_*.c) \
		$(COVOBJ_a02) -o $@

.PHONY: cover-a02
cover-a02: $(COVBIN_a02)
	@find $(COVDIR_a02) -name '*.gcda' -delete
	@for t in $(COVBIN_a02); do $$t >/dev/null || exit 1; done
	@cd $(COVDIR_a02) && for s in $(KSRC_a02); do \
		LC_ALL=C gcov -b -g -o $(CURDIR)/$(COVDIR_a02)/$$(dirname $$s) \
			$(CURDIR)/$$s 2>/dev/null; done | \
		grep -A5 "^File '.*kernel/mm/pmm_[a-z_]*\.[ch]'" | \
		grep -E "^File|^Lines|^Branches|^Taken|^Condition" | \
		sed -e "s|^File '.*/\(pmm_[a-z_]*\.[ch]\)'|== \1|" \
			-e 's|^Lines executed:\([0-9.]*\)% of \([0-9]*\)|lignes \1 % (\2)|' \
			-e 's|^Branches executed:\([0-9.]*\)%.*|branches executees \1 %|' \
			-e 's|^Taken at least once:\([0-9.]*\)%.*|branches prises \1 %|' \
			-e 's|^Condition outcomes covered:\([0-9.]*\)%.*|conditions \1 %|'
