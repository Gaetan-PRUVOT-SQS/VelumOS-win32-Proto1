#include "r3.h"

uint64_t	r3_stack_base(uint64_t size)
{
	uint64_t	q[2];
	uint64_t	here;

	here = (uint64_t)q;
	if (r3_sys(SYS_VQUERY, here, (uint64_t)q, 0) != 0)
		return (0);
	return ((here & ~(R3_PAGE - 1)) + q[1] - size);
}

int	r3_stack_child(void)
{
	uint64_t	base;

	base = r3_stack_base(R3_STACK_SIZE);
	if (!base)
		return (R3_NO_BASE);
	if (r3_sys(SYS_VALLOC, base - R3_PAGE, R3_PAGE, PROT_R | PROT_W) > 0)
		return (R3_NO_GUARD);
	r3_do_overflow();
	return (R3_NO_FAULT);
}

static uint32_t	r3_nprocs(void)
{
	t_sysinfo	si;

	memset(&si, 0, sizeof(si));
	if (r3_sys(SYS_SYSINFO, (uint64_t)(&si), 0, 0) != 0)
		return (0);
	return (si.nprocs);
}

static void	r3_count(t_r3 *t)
{
	t_procinfo	info;
	int64_t		h;
	uint32_t	before;
	uint32_t	polls;

	before = r3_nprocs();
	r3_check(t, "sysinfo compte au moins ce processus", before >= 1, 1);
	h = r3_child("dort", PF_ALL);
	r3_check(t, "enfant compté lancé", h > 0, 1);
	if (h <= 0)
		return ;
	r3_check(t, "nprocs compte l'enfant vivant", r3_nprocs(), before + 1);
	r3_sys(SYS_PROC_KILL, h, 7, 0);
	r3_reap(h, &info);
	polls = 0;
	while (r3_nprocs() != before && polls++ < R3_POLLS)
		r3_sys(SYS_SLEEP, R3_NAP_NS / 10, 0, 0);
	r3_check(t, "nprocs revenu après la récolte", r3_nprocs(), before);
}

void	r3_suite_stack(t_r3 *t)
{
	t_procinfo	info;
	uint64_t	base;
	int64_t		h;

	base = r3_stack_base(R3_STACK_SIZE);
	r3_check(t, "pile : base trouvée", base != 0, 1);
	r3_check(t, "pile : mappage sur la garde refusé", r3_sys(SYS_VALLOC,
			base - R3_PAGE, R3_PAGE, PROT_R | PROT_W), E_EXIST);
	h = r3_child("pile", PF_ALL);
	r3_check(t, "pile : enfant lancé", h > 0, 1);
	if (h > 0 && r3_reap(h, &info) == 0)
		r3_check(t, "pile : débordement tué par faute de page",
			(int32_t)info.reserved, -1);
	else
		r3_check(t, "pile : enfant attendu", 0, 1);
	r3_count(t);
	r3_suite_guard(t, base - R3_PAGE);
	r3_suite_held(t, base);
}
