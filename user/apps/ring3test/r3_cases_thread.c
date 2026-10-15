#include "r3.h"

static volatile uint64_t	g_r3_shared;

uint64_t	r3_shared(void)
{
	return (g_r3_shared);
}

void	r3_hold_main(uint64_t arg)
{
	*(volatile uint64_t *)arg = r3_stack_base(R3_TSTACK_SIZE);
	while (1)
		r3_sys(SYS_SLEEP, R3_NAP_NS, 0, 0);
}

void	r3_thread_main(uint64_t arg)
{
	*(volatile uint64_t *)arg = R3_MAGIC;
	while (1)
		r3_sys(SYS_THREAD_EXIT, 5, 0, 0);
}

int64_t	r3_thread_raw(uint64_t entry, uint64_t stack, uint64_t prio,
		uint64_t flags)
{
	uint64_t			a[6];
	volatile uint64_t	*shared;

	shared = &g_r3_shared;
	a[0] = entry;
	a[1] = (uint64_t)shared;
	a[2] = stack;
	a[3] = prio;
	a[4] = flags;
	a[5] = 0;
	return (v_syscall6(SYS_THREAD_CREATE, a));
}

void	r3_suite_thread(t_r3 *t)
{
	int64_t	h;

	g_r3_shared = 0;
	h = r3_thread_raw((uint64_t)r3_thread_tramp, 0, R3_PRIO, 0);
	r3_check(t, "fil utilisateur créé", h > 0, 1);
	if (h <= 0)
		return ;
	r3_check(t, "fil attendu", r3_sys(SYS_WAIT, h, R3_WAIT_NS, 0), 0);
	r3_check(t, "fil a écrit sa valeur", g_r3_shared, R3_MAGIC);
	r3_sys(SYS_CLOSE, h, 0, 0);
}
