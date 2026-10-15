#include <string.h>
#include "fake.h"

static bool	fk_bad_range(t_uptr p, size_t n)
{
	uint64_t	end;

	if (!p || __builtin_add_overflow(p, n, &end))
		return (true);
	return (g_fk.fault_hi && p < g_fk.fault_hi && end > g_fk.fault_lo);
}

int	copy_from_user(void *dst, t_uptr src, size_t n)
{
	if (n == 0)
		return (0);
	if (fk_bad_range(src, n))
		return (E_FAULT);
	memcpy(dst, (const void *)(uintptr_t)src, n);
	return (0);
}

int	copy_to_user(t_uptr dst, const void *src, size_t n)
{
	if (n == 0)
		return (0);
	if (fk_bad_range(dst, n))
		return (E_FAULT);
	memcpy((void *)(uintptr_t)dst, src, n);
	return (0);
}
