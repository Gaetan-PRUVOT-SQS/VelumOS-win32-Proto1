#include "r3.h"

static int	r3_seq_refusals(uint64_t base)
{
	if (r3_sys(SYS_VFREE, base, R3_TSTACK_SIZE, 0) != E_ACCES)
		return (R3_STACK_FREED);
	if (r3_sys(SYS_VFREE, base + R3_PAGE, R3_PAGE, 0) != E_ACCES)
		return (R3_STACK_FREED);
	if (r3_sys(SYS_VFREE, base - R3_PAGE, R3_TSTACK_SIZE + R3_PAGE, 0)
		!= E_ACCES)
		return (R3_STACK_FREED);
	if (r3_sys(SYS_VPROTECT, base, R3_PAGE, PROT_R) != E_ACCES)
		return (R3_STACK_PROT);
	return (0);
}

int	r3_seq_child(void)
{
	uint64_t	base;
	int			polls;
	int			rc;

	if (r3_thread_raw((uint64_t)r3_hold_tramp, 0, R3_PRIO, 0) <= 0)
		return (R3_NO_BASE);
	polls = 0;
	while (!r3_shared() && polls++ < R3_POLLS)
		r3_sys(SYS_SLEEP, R3_NAP_NS / 10, 0, 0);
	base = r3_shared();
	if (!base)
		return (R3_NO_BASE);
	rc = r3_seq_refusals(base);
	if (rc)
		return (rc);
	return (r3_guard_child());
}

void	r3_suite_held(t_r3 *t, uint64_t base)
{
	t_procinfo	info;
	uint64_t	q[2];
	int64_t		h;

	r3_check(t, "pile tenue : vfree d'une page refusé", r3_sys(SYS_VFREE,
			base, R3_PAGE, 0), E_ACCES);
	r3_check(t, "pile tenue : vfree avec la garde refusé", r3_sys(SYS_VFREE,
			base - R3_PAGE, 2 * R3_PAGE, 0), E_ACCES);
	r3_check(t, "pile tenue : vprotect refusé", r3_sys(SYS_VPROTECT, base,
			R3_PAGE, PROT_R), E_ACCES);
	r3_sys(SYS_VQUERY, base, (uint64_t)q, 0);
	r3_check(t, "pile tenue : décrite en lecture et écriture",
		q[0] & 0xffffffffu, PROT_R | PROT_W);
	h = r3_child("pileseq", PF_ALL);
	if (h > 0 && r3_reap(h, &info) == 0)
		r3_check(t, "pile tenue : séquence vfree puis fil, garde intacte",
			(int32_t)info.reserved, -1);
	else
		r3_check(t, "pile tenue : enfant de séquence attendu", 0, 1);
}
