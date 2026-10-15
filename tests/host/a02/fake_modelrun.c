#include "a02_fake.h"

static uint64_t	pick_max(void)
{
	uint64_t	r;

	r = fake_below(100);
	if (r < 80)
		return (0);
	if (r < 88)
		return (MIB);
	if (r < 94)
		return (2 * MIB);
	return (PAGE_SIZE + fake_below(g_pmm.span * PAGE_SIZE * 2));
}

static void	pick_request(t_mreq *rq)
{
	rq->n = 1;
	if (fake_below(100) < 40)
		rq->n = 2 + fake_below(69);
	rq->al = 1;
	if (fake_below(100) < 30)
		rq->al = 1ull << (1 + fake_below(9));
	rq->max = pick_max();
}

void	fake_run_track(t_run *r, uint64_t phys, uint64_t n, t_pmm_owner o)
{
	r->live[r->nlive].phys = phys;
	r->live[r->nlive].n = n;
	r->live[r->nlive].owner = o;
	r->nlive++;
	r->frames += n;
}

void	fake_run_alloc(t_run *r)
{
	t_mreq		rq;
	t_pmm_owner	o;
	uint64_t	phys;

	pick_request(&rq);
	o = (t_pmm_owner)(PMM_KERNEL + fake_below(PMM_OWNERS - 1));
	phys = pmm_alloc_pages(o, rq.n, rq.al, rq.max);
	r->allocs++;
	r->refused += !phys;
	r->errors += (!phys && model_feasible(&rq));
	if (!phys)
		return ;
	r->errors += !model_alloc_ok(&rq, phys, o);
	fake_run_track(r, phys, rq.n, o);
}

void	fake_run_free(t_run *r)
{
	t_farg		*b;
	uint64_t	k;

	if (!r->nlive)
		return ;
	b = &r->live[fake_below(r->nlive)];
	k = 1 + fake_below(b->n);
	r->errors += !model_free(b->phys, k, b->owner);
	pmm_free_pages(b->phys, k, b->owner);
	b->phys += k * PAGE_SIZE;
	b->n -= k;
	r->frames -= k;
	r->frees++;
	if (!b->n)
		*b = r->live[--r->nlive];
}
