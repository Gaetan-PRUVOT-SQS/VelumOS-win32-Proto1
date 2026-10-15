#include "fake_check.h"
#include "fake_sys.h"
#include "harness.h"

void	sys_is(uint64_t num, uint64_t a0, uint64_t a1, uint64_t a2)
{
	h_eq_u64("numero d'appel", g_fsys.last_num, num);
	h_eq_u64("argument 0", g_fsys.last_args[0], a0);
	h_eq_u64("argument 1", g_fsys.last_args[1], a1);
	h_eq_u64("argument 2", g_fsys.last_args[2], a2);
}

void	sys_is_hi(uint64_t a3, uint64_t a4, uint64_t a5)
{
	h_eq_u64("argument 3", g_fsys.last_args[3], a3);
	h_eq_u64("argument 4", g_fsys.last_args[4], a4);
	h_eq_u64("argument 5", g_fsys.last_args[5], a5);
}

void	sys_none(void)
{
	h_eq_u64("aucun appel", g_fsys.calls, 0);
}

uint64_t	sys_u(const void *p)
{
	return ((uint64_t)(uintptr_t)p);
}
