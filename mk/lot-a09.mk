LOTS += a09
KSRC_a09 := $(sort $(wildcard lib/crypto/*.c kernel/random/*.c \
	drivers/pci/*.c kernel/random/*.S)) \
	$(if $(A09_STUBS),tests/host/a09/kstub/stubs.c)
DIRS_a09 := lib/crypto kernel/random drivers/pci tests/host/a09 \
	include/velum/pci.h include/velum/random.h include/velum/crypto.h
HT_a09 := $(sort $(wildcard tests/host/a09/test_*.c))
HSRC_a09 := $(sort $(wildcard lib/crypto/*.c)) \
	$(filter-out kernel/random/rng_plat.c,\
	$(sort $(wildcard kernel/random/*.c))) \
	$(filter-out drivers/pci/pci_legacy.c,\
	$(sort $(wildcard drivers/pci/*.c))) \
	kernel/bootinfo.c lib/libk/str.c lib/libk/str2.c lib/libk/memcmp.c
HINC_a09 := -Itests/host/a09 -Ilib/crypto -Ikernel/random -Idrivers/pci
ULIBS += crypto
ULIB_crypto_LOT := a09
ULIB_crypto_SRC := $(filter-out lib/crypto/chacha20%,\
	$(sort $(wildcard lib/crypto/*.c)))
