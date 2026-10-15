#include "velum/err.h"
#include "vmm_int.h"

int	copy_to_user(t_uptr dst, const void *src, size_t n)
{
	t_ucopy	c;

	c.kbuf = (uint8_t *)(uintptr_t)src;
	c.uva = dst;
	c.n = n;
	c.to_user = true;
	return (vmm_user_xfer(&c));
}

bool	user_range_ok(t_uptr addr, size_t n)
{
	t_aspace	*as;
	uint64_t	irq;
	int			rc;

	if (!n)
		return (true);
	as = g_vmm.current;
	if (!as)
		return (false);
	irq = vmm_lock(as);
	rc = vmm_user_check(as, addr, n, false);
	vmm_unlock(as, irq);
	return (rc == 0);
}

static size_t	copy_until_nul(char *dst, const char *src, size_t n)
{
	size_t	i;

	i = 0;
	while (i < n)
	{
		dst[i] = src[i];
		if (!src[i])
			return (i);
		i++;
	}
	return (n);
}

static int	ustr_copy(t_aspace *as, char *dst, t_uptr src, size_t max)
{
	t_ptlook	look;
	uintptr_t	va;
	size_t		i;
	size_t		chunk;
	size_t		got;

	i = 0;
	while (i < max)
	{
		if (__builtin_add_overflow(src, i, &va)
			|| !vmm_user_page(as, va, false, &look))
			return (E_FAULT);
		chunk = min_u64(max - i, PAGE_SIZE - (va & (PAGE_SIZE - 1)));
		got = copy_until_nul(dst + i, phys_to_virt(look.pa), chunk);
		if (got < chunk)
			return ((int)(i + got));
		i += chunk;
	}
	return (E_RANGE);
}

int	strncpy_from_user(char *dst, t_uptr src, size_t max)
{
	t_aspace	*as;
	uint64_t	irq;
	int			rc;

	if (!max)
		return (E_RANGE);
	if (!dst)
		return (E_FAULT);
	dst[0] = '\0';
	as = g_vmm.current;
	if (!as)
		return (E_FAULT);
	if (max > 0x7fffffff)
		max = 0x7fffffff;
	irq = vmm_lock(as);
	rc = ustr_copy(as, dst, src, max);
	vmm_unlock(as, irq);
	if (rc < 0)
		dst[0] = '\0';
	return (rc);
}
