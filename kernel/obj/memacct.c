#include "obj_int.h"
#include "velum/heap.h"

t_memacct	*acct_new(void)
{
	t_memacct	*a;

	a = kmalloc_tag(sizeof(t_memacct), HEAP_OBJECT);
	if (!a)
		return (NULL);
	a->bytes = 0;
	a->refs = 1;
	a->pad = 0;
	return (a);
}

void	acct_ref(t_memacct *a)
{
	__atomic_fetch_add(&a->refs, 1, __ATOMIC_RELAXED);
}

void	acct_unref(t_memacct *a)
{
	if (!a)
		return ;
	if (__atomic_fetch_sub(&a->refs, 1, __ATOMIC_ACQ_REL) == 1)
		kfree(a);
}

int	acct_charge(t_memacct *a, uint64_t used, uint64_t lim, uint64_t n)
{
	uint64_t	now;

	now = __atomic_add_fetch(&a->bytes, n, __ATOMIC_ACQ_REL);
	if (now >= n && used <= lim && now <= lim - used)
		return (0);
	__atomic_sub_fetch(&a->bytes, n, __ATOMIC_ACQ_REL);
	return (E_NOMEM);
}

void	acct_uncharge(t_memacct *a, uint64_t bytes)
{
	if (a)
		__atomic_sub_fetch(&a->bytes, bytes, __ATOMIC_ACQ_REL);
}
