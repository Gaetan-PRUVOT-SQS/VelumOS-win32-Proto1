#include "velum/err.h"
#include "velum/libk.h"
#include "vmm_int.h"

bool	vmm_user_page(t_aspace *as, uintptr_t va, bool wr, t_ptlook *lk)
{
	if (va < USER_MIN || va >= USER_TOP)
		return (false);
	if (!vmm_lookup(as, va, lk))
		return (false);
	if (!(lk->pte & PTE_U) || (wr && !(lk->pte & PTE_W)))
		return (false);
	return (vmm_hhdm_covers(lk->pa, 1));
}

int	vmm_user_check(t_aspace *as, t_uptr a, size_t n, bool wr)
{
	t_ptlook	look;
	uintptr_t	va;
	uintptr_t	end;

	if (__builtin_add_overflow(a, n, &end) || a < USER_MIN || end > USER_TOP)
		return (E_FAULT);
	va = align_down(a, PAGE_SIZE);
	while (va < end)
	{
		if (!vmm_user_page(as, va, wr, &look))
			return (E_FAULT);
		va += PAGE_SIZE;
	}
	return (0);
}

void	vmm_user_copy(t_aspace *as, const t_ucopy *c)
{
	t_ptlook	look;
	size_t		done;
	size_t		chunk;
	uint8_t		*user;

	done = 0;
	while (done < c->n)
	{
		vmm_lookup(as, c->uva + done, &look);
		chunk = min_u64(c->n - done,
				PAGE_SIZE - ((c->uva + done) & (PAGE_SIZE - 1)));
		user = phys_to_virt(look.pa);
		if (c->to_user)
			memcpy(user, c->kbuf + done, chunk);
		else
			memcpy(c->kbuf + done, user, chunk);
		done += chunk;
	}
}

int	vmm_user_xfer(const t_ucopy *c)
{
	t_aspace	*as;
	uint64_t	irq;
	int			rc;

	if (!c->n)
		return (0);
	as = g_vmm.current;
	if (!as || !c->kbuf)
		return (E_FAULT);
	irq = vmm_lock(as);
	rc = vmm_user_check(as, c->uva, c->n, c->to_user);
	if (rc == 0)
		vmm_user_copy(as, c);
	vmm_unlock(as, irq);
	return (rc);
}

int	copy_from_user(void *dst, t_uptr src, size_t n)
{
	t_ucopy	c;

	c.kbuf = dst;
	c.uva = src;
	c.n = n;
	c.to_user = false;
	return (vmm_user_xfer(&c));
}
