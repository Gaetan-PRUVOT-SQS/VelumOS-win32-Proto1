BENCH_GLASS_BIN := $(B)/bench/glass/bench_glass
BENCH_GLASS_SRC := $(sort $(wildcard lib/luna/*.c)) lib/libk/str.c \
	$(sort $(wildcard tests/host/a17/fake_*.c)) tests/host/a17/bench_glass.c
BENCH_GLASS_FLAGS := $(filter-out -O1 -fsanitize=% -fno-sanitize-recover=%,$(HOSTFLAGS)) \
	-O2 -fwrapv -fno-strict-aliasing -fno-omit-frame-pointer \
	-fstack-protector-strong -ftrivial-auto-var-init=zero \
	-DVELUM_BENCH_GLASS $(HINC_a17)
BENCH_GLASS_PASSES ?= 9

$(BENCH_GLASS_BIN): $(BENCH_GLASS_SRC) $(wildcard lib/luna/*.h) $(wildcard include/velum/*.h)
	@mkdir -p $(dir $@)
	$(HOSTCC) $(BENCH_GLASS_FLAGS) $(BENCH_GLASS_SRC) -o $@

.PHONY: bench-glass
bench-glass: $(BENCH_GLASS_BIN)
	@$(BENCH_GLASS_BIN) $(BENCH_GLASS_PASSES)
