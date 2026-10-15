LOTS += a04
KSRC_a04 := $(sort $(wildcard kernel/mm/heap*.c kernel/mm/slab*.c))
DIRS_a04 := $(KSRC_a04) $(sort $(wildcard kernel/mm/heap*.h kernel/mm/slab*.h)) \
	include/velum/heap.h tests/host/a04
HSRC_a04 := $(filter-out kernel/mm/heap_pages.c,$(KSRC_a04)) \
	lib/libk/str.c lib/libk/str2.c lib/libk/memcmp.c
HINC_a04 := -Ikernel/mm -DVELUM_DEBUG

A04_TESTS := $(sort $(wildcard tests/host/a04/test_*.c))
A04_FAKES := $(sort $(wildcard tests/host/a04/fake_*.c))
A04_BASE := tests/host/harness.c tests/host/harness_eq.c
A04_SRC := $(HSRC_a04) $(A04_BASE) $(A04_FAKES)
A04_DEPS := $(wildcard kernel/mm/heap*.h kernel/mm/slab*.h include/velum/heap.h \
	tests/host/a04/*.h)
A04_DIR := $(B)/host/a04
A04_OBJ_dbg := $(patsubst %.c,$(A04_DIR)/o/dbg/%.o,$(A04_SRC))
A04_OBJ_rel := $(patsubst %.c,$(A04_DIR)/o/rel/%.o,$(A04_SRC))
A04_CFLAGS_dbg := $(HINC_a04)
A04_CFLAGS_rel := $(HINC_a04) -UVELUM_DEBUG -DNDEBUG

define a04_profile
$(A04_DIR)/o/$(1)/%.o: %.c $$(A04_DEPS)
	@mkdir -p $$(dir $$@)
	$$(HOSTCC) $$(HOSTFLAGS) $$(A04_CFLAGS_$(1)) -c $$< -o $$@

$(A04_DIR)/$(1)/%: tests/host/a04/%.c $$(A04_OBJ_$(1)) $$(A04_DEPS)
	@mkdir -p $$(dir $$@)
	$$(HOSTCC) $$(HOSTFLAGS) $$(A04_CFLAGS_$(1)) $$< $$(A04_OBJ_$(1)) -o $$@

HBIN_a04 += $$(patsubst tests/host/a04/%.c,$(A04_DIR)/$(1)/%,$$(A04_TESTS))
endef

$(eval $(call a04_profile,dbg))
$(eval $(call a04_profile,rel))

.PHONY: cov-a04 bench-a04
cov-a04:
	sh tests/host/a04/couverture.sh $(B)

bench-a04:
	sh tests/host/a04/mesure.sh $(B)

A04_COMMA := ,
A04_TSAN = $(filter-out -fsanitize=address$(A04_COMMA)undefined \
	-fno-sanitize-recover=undefined,$(HOSTFLAGS)) -fsanitize=thread

.PHONY: tsan-a04
tsan-a04:
	@mkdir -p $(B)/tsan
	@for t in test_concurrent test_lock; do \
		$(HOSTCC) $(A04_TSAN) $(HINC_a04) tests/host/a04/$$t.c $(A04_SRC) \
			-o $(B)/tsan/$$t && $(B)/tsan/$$t || exit 1; \
	done
