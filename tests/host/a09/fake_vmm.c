#include <stdlib.h>
#include <string.h>
#include "fake.h"
#include "velum/err.h"
#include "velum/vmm.h"

static int	user_range_inside(uint64_t addr, size_t n)
{
	if (!g_fake.user_mem || addr < g_fake.user_base)
		return (0);
	if (addr - g_fake.user_base > g_fake.user_len)
		return (0);
	return (n <= g_fake.user_len - (addr - g_fake.user_base));
}

int	copy_to_user(t_uptr dst, const void *src, size_t n)
{
	if (!n)
		return (0);
	if (!user_range_inside(dst, n))
		return (E_FAULT);
	memcpy(g_fake.user_mem + (dst - g_fake.user_base), src, n);
	return (0);
}

int	copy_from_user(void *dst, t_uptr src, size_t n)
{
	if (!n)
		return (0);
	if (!user_range_inside(src, n))
		return (E_FAULT);
	memcpy(dst, g_fake.user_mem + (src - g_fake.user_base), n);
	return (0);
}

bool	user_range_ok(t_uptr addr, size_t n)
{
	return (user_range_inside(addr, n));
}
