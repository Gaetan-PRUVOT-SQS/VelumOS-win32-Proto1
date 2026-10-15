#include "r3.h"

void	r3_ovf_main(void)
{
	uint64_t	base;

	base = r3_stack_base(R3_TSTACK_SIZE);
	if (!base)
		r3_sys(SYS_EXIT, R3_NO_BASE, 0, 0);
	if (r3_sys(SYS_VALLOC, base - R3_PAGE, R3_PAGE, PROT_R | PROT_W) > 0)
		r3_sys(SYS_EXIT, R3_NO_GUARD, 0, 0);
	r3_do_overflow();
}

int	r3_guard_child(void)
{
	if (r3_thread_raw((uint64_t)r3_ovf_tramp, 0, R3_PRIO, 0) <= 0)
		return (R3_NO_BASE);
	while (1)
		r3_sys(SYS_SLEEP, R3_NAP_NS, 0, 0);
	return (R3_NO_FAULT);
}

static int64_t	r3_layout(void)
{
	uint64_t	q[2];
	uint64_t	va;
	int64_t		steps;

	va = R3_USER_MIN;
	steps = 0;
	while (va < R3_USER_TOP && steps < R3_WALK_MAX)
	{
		if (r3_sys(SYS_VQUERY, va, (uint64_t)q, 0) != 0 || q[1] == 0)
			return (-1);
		va += q[1];
		steps++;
	}
	return (steps);
}

static int	r3_threads(void)
{
	int64_t	h;
	int		made;
	int		tries;

	made = 0;
	tries = 0;
	while (made < R3_THREADS && tries++ < 4 * R3_THREADS)
	{
		h = r3_thread_raw((uint64_t)r3_thread_tramp, R3_TSTACK_MIN,
				R3_PRIO, 0);
		if (h > 0 && r3_sys(SYS_WAIT, h, R3_WAIT_NS, 0) == 0)
			made++;
		if (h > 0)
			r3_sys(SYS_CLOSE, h, 0, 0);
		else
			r3_sys(SYS_SLEEP, R3_NAP_NS / 10, 0, 0);
	}
	return (made);
}

void	r3_suite_guard(t_r3 *t, uint64_t guard)
{
	t_procinfo	info;
	uint64_t	q[2];
	int64_t		n;

	r3_check(t, "garde : vfree refusé", r3_sys(SYS_VFREE, guard, R3_PAGE, 0),
		E_ACCES);
	r3_check(t, "garde : vprotect refusé", r3_sys(SYS_VPROTECT, guard,
			R3_PAGE, PROT_R | PROT_W), E_NOENT);
	r3_sys(SYS_VQUERY, guard, (uint64_t)q, 0);
	r3_check(t, "garde : décrite sans droit", q[0] & 0xffffffffu, 0);
	r3_check(t, "garde : toujours là après les refus", r3_sys(SYS_VALLOC,
			guard, R3_PAGE, PROT_R | PROT_W), E_EXIST);
	n = r3_layout();
	r3_check(t, "garde : 1000 fils créés et finis", r3_threads(), R3_THREADS);
	r3_sys(SYS_SLEEP, R3_NAP_NS, 0, 0);
	r3_check(t, "garde : régions revenues au départ", r3_layout(), n);
	n = r3_child("pilefil", PF_ALL);
	if (n > 0 && r3_reap(n, &info) == 0)
		r3_check(t, "garde : débordement d'un fil tue le processus",
			(int32_t)info.reserved, -1);
	else
		r3_check(t, "garde : enfant à fil attendu", 0, 1);
}
