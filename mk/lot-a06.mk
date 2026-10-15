LOTS += a06
KSRC_a06 := $(sort $(wildcard kernel/sched/*.c)) arch/x86_64/switch_ctx.c \
	arch/x86_64/switch.S
DIRS_a06 := kernel/sched arch/x86_64/switch_ctx.c include/velum/sched.h \
	include/velum/sync.h tests/host/a06
HT_a06 := $(sort $(wildcard tests/host/a06/test_*.c))
HSRC_a06 := kernel/sched/spin.c kernel/sched/spin_rel.c kernel/sched/lockdep.c \
	kernel/sched/lockdep_stack.c kernel/sched/waitq_list.c \
	kernel/sched/waitq.c kernel/sched/waitq_tmo.c kernel/sched/waitq_wake.c \
	kernel/sched/mutex.c kernel/sched/event.c kernel/sched/sem.c \
	kernel/sched/runq.c kernel/sched/runq_ops.c kernel/sched/policy.c \
	kernel/sched/policy_pick.c lib/libk/str.c lib/libk/str2.c \
	lib/libk/memcmp.c
HINC_a06 := -Ikernel/sched -Itests/host/a06 -DVELUM_DEBUG -pthread
