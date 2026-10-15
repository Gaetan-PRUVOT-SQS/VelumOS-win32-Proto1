LOTS += a11
KSRC_a11 := $(sort $(wildcard drivers/block/*.c)) \
	$(sort $(wildcard drivers/virtio/*.c))
DIRS_a11 := drivers/block drivers/virtio tests/host/a11
HT_a11 := $(sort $(wildcard tests/host/a11/test_*.c))
HSRC_a11 := $(filter-out drivers/virtio/virtio_io%.c,$(KSRC_a11)) \
	lib/libk/str.c lib/libk/str2.c lib/libk/memcmp.c
HINC_a11 := -Idrivers/block -Idrivers/virtio -Itests/host/a11
