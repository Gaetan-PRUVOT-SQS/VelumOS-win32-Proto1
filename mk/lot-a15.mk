LOTS += a15
KSRC_a15 := $(sort $(wildcard lib/gfx/*.c))
DIRS_a15 := lib/gfx tests/host/a15 include/velum/gfx.h
HT_a15 := $(sort $(wildcard tests/host/a15/test_*.c))
HSRC_a15 := $(KSRC_a15) tests/host/a15/ppm.c lib/libk/str.c lib/libk/str2.c \
	lib/libk/memcmp.c
HINC_a15 := -Ilib/gfx -Itests/host/a15
ULIBS += gfx
ULIB_gfx_LOT := a15
ULIB_gfx_SRC := $(sort $(wildcard lib/gfx/*.c))

BENCHSRC_a15 := $(sort $(wildcard tests/host/a15/bench*.c))
BENCHFLAGS_a15 := -std=gnu11 -O2 -Wall -Wextra -Werror -Iinclude -Ilib/libk \
	-Ilib/gfx -Itests/host/a15 -DVELUM_HOST

$(B)/bench/a15_bench: $(BENCHSRC_a15) $(KSRC_a15) $(wildcard tests/host/a15/*.h)
	@mkdir -p $(dir $@)
	$(HOSTCC) $(BENCHFLAGS_a15) $(BENCHSRC_a15) $(KSRC_a15) -o $@

$(B)/bench/a15_bench_k: $(BENCHSRC_a15) $(KSRC_a15) $(wildcard tests/host/a15/*.h)
	@mkdir -p $(dir $@)
	$(HOSTCC) $(BENCHFLAGS_a15) -mgeneral-regs-only $(BENCHSRC_a15) \
		$(KSRC_a15) -o $@

.PHONY: bench-a15 bench-a15-k
bench-a15: $(B)/bench/a15_bench
	$<

bench-a15-k: $(B)/bench/a15_bench_k
	$<
