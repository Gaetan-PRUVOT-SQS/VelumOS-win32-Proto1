LOTS += a13
KSRC_a13 := $(sort $(wildcard drivers/display/*.c)) \
	$(sort $(wildcard kernel/kcon/*.c))
DIRS_a13 := drivers/display kernel/kcon tests/host/a13 include/velum/display.h
HT_a13 := $(sort $(wildcard tests/host/a13/test_*.c))
HSRC_a13 := $(sort $(wildcard kernel/kcon/*.c)) \
	$(filter-out drivers/display/dispi_hw.c drivers/display/dispi_io.c \
	drivers/display/display_selftest%.c, \
	$(sort $(wildcard drivers/display/*.c))) \
	kernel/bootinfo.c lib/libk/str.c lib/libk/str2.c lib/libk/memcmp.c
HINC_a13 := -Idrivers/display -Ikernel/kcon -Itests/host/a13 -DVELUM_DEBUG

.PHONY: cov-a13
cov-a13:
	python3 tests/host/a13/cov.py --out $(B)/cov --hostcc "$(HOSTCC)" \
		--tests "$(HT_a13)" --sources "$(HSRC_a13)" \
		--flags "$(HOSTFLAGS) $(HINC_a13)"
