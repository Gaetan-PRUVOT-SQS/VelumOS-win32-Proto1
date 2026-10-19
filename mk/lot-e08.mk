LOTS += e08
KSRC_e08 :=
DIRS_e08 := kernel/sched tests/host/e08
HT_e08 := $(sort $(wildcard tests/host/e08/test_*.c))
HSRC_e08 := $(HSRC_a06) kernel/sched/smp_steal.c kernel/sched/smp_place.c \
	$(sort $(wildcard tests/host/a06/fake_*.c))
HINC_e08 := -Ikernel/sched -Itests/host/a06 -Itests/host/e08 -pthread
