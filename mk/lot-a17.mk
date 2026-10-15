LOTS += a17
KSRC_a17 := $(sort $(wildcard lib/luna/*.c))
DIRS_a17 := lib/luna tests/host/a17 include/velum/luna.h
HT_a17 := $(sort $(wildcard tests/host/a17/test_*.c))
HSUP_a17 := $(wildcard tests/host/a17/test_*.c tests/host/a17/fake_*.c)
HSRC_a17 := $(sort $(wildcard lib/luna/*.c)) lib/libk/str.c \
	$(filter-out $(HSUP_a17),$(sort $(wildcard tests/host/a17/*.c)))
HINC_a17 := -Ilib/luna -Itests/host/a17
ULIBS += luna
ULIB_luna_LOT := a17
ULIB_luna_SRC := $(sort $(wildcard lib/luna/*.c))
