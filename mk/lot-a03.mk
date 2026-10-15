LOTS += a03
A03_KONLY := kernel/mm/vmm_addr.c kernel/mm/vmm_boot.c kernel/mm/vmm_boot2.c \
	kernel/mm/vmm_cpu.c kernel/mm/vmm_fault.c kernel/mm/vmm_selftest.c \
	kernel/mm/vmm_selftest2.c kernel/mm/vmm_sys.c kernel/mm/vmm_sys2.c
KSRC_a03 := $(sort $(wildcard kernel/mm/vmm*.c)) arch/x86_64/mmu.S
DIRS_a03 := $(sort $(wildcard kernel/mm/vmm*)) include/velum/vmm.h tests/host/a03
HT_a03 := $(sort $(wildcard tests/host/a03/test_*.c))
HSRC_a03 := $(filter-out $(A03_KONLY),$(sort $(wildcard kernel/mm/vmm*.c))) \
	lib/libk/str.c lib/libk/memcmp.c
HINC_a03 := -Ikernel/mm


COVDIR_a03 := $(B)/cover/a03
COVSRC_a03 := $(filter kernel/mm/%,$(HSRC_a03))
COVOBJ_a03 := $(patsubst %.c,$(COVDIR_a03)/%.o,$(HSRC_a03))
COVBIN_a03 := $(patsubst tests/host/a03/%.c,$(COVDIR_a03)/bin/%,$(HT_a03))
COVFLAGS_a03 := -O0 --coverage -fcondition-coverage -fprofile-abs-path

$(COVDIR_a03)/%.o: %.c
	@mkdir -p $(dir $@)
	$(HOSTCC) $(HOSTFLAGS) $(COVFLAGS_a03) $(HINC_a03) -c $< -o $@

$(COVDIR_a03)/bin/%: tests/host/a03/%.c $(COVOBJ_a03) \
		$(wildcard tests/host/a03/fake_*.c) $(wildcard tests/host/a03/*.h)
	@mkdir -p $(dir $@)
	$(HOSTCC) $(HOSTFLAGS) -O0 --coverage $(HINC_a03) $< tests/host/harness.c \
		tests/host/harness_eq.c $(wildcard tests/host/a03/fake_*.c) \
		$(COVOBJ_a03) -o $@

.PHONY: cover-a03
cover-a03: $(COVBIN_a03)
	@find $(COVDIR_a03) -name '*.gcda' -delete
	@for t in $(COVBIN_a03); do $$t >/dev/null || exit 1; done
	@cd $(COVDIR_a03) && for s in $(COVSRC_a03); do \
		LC_ALL=C gcov -b -g -o $(CURDIR)/$(COVDIR_a03)/$$(dirname $$s) \
			$(CURDIR)/$$s 2>/dev/null; done | \
		grep -A5 "^File '.*kernel/mm/vmm_[a-z0-9_]*\.c'" | \
		grep -E "^File|^Lines|^Branches|^Condition"
