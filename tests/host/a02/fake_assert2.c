#include "a02_fake.h"

static void	run_free(void *arg)
{
	const t_farg	*a;

	a = arg;
	pmm_free_pages(a->phys, a->n, a->owner);
}

int	fake_free_try(uint64_t phys, uint64_t n, t_pmm_owner o)
{
	t_farg	a;

	a.phys = phys;
	a.n = n;
	a.owner = o;
	return (fake_catch(run_free, &a));
}
