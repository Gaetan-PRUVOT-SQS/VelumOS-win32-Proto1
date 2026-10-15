#include "a02_fake.h"

static void	run_zero(t_run *r)
{
	t_mreq		rq;
	uint64_t	phys;

	rq.n = 1;
	rq.al = 1;
	rq.max = 0;
	phys = pmm_alloc_zero(PMM_USER);
	r->allocs++;
	r->refused += !phys;
	if (!phys)
		return ;
	r->errors += !model_alloc_ok(&rq, phys, PMM_USER);
	r->errors += !pmm_st_frame_is(phys, 1, 0);
	fake_run_track(r, phys, 1, PMM_USER);
}

static void	run_check(t_run *r)
{
	t_pmm_stats	s;
	int			o;

	pmm_get_stats(&s);
	r->errors += (model_diff() != 0);
	r->errors += (pmm_check() != 0);
	r->errors += (model_count(PMM_FREE) != s.free_pages);
	o = PMM_KERNEL;
	while (o < PMM_OWNERS)
	{
		r->errors += (model_count(o) != s.owned[o]);
		o++;
	}
}

void	fake_model_run(t_run *r, uint64_t ops)
{
	uint64_t	i;
	uint64_t	roll;

	i = 0;
	while (i < ops)
	{
		roll = fake_below(100);
		if (roll < 50 && r->frames * 10 < g_pmm.span * 6)
			fake_run_alloc(r);
		else if (roll < 92)
			fake_run_free(r);
		else
			run_zero(r);
		i++;
		if (i % 1000 == 0)
			run_check(r);
	}
	run_check(r);
}

int	fake_model_machine(void)
{
	fake_reset();
	fake_range(0, 3 * MIB, MEM_USABLE);
	fake_range(3 * MIB, 256 * 1024, MEM_RESERVED);
	fake_range(3 * MIB + 256 * 1024, 4 * MIB + 768 * 1024, MEM_USABLE);
	fake_range(8 * MIB, MIB, MEM_BOOT_RECLAIM);
	return (fake_boot());
}

void	fake_model_drain(t_run *r)
{
	while (r->nlive)
	{
		r->nlive--;
		r->errors += !model_free(r->live[r->nlive].phys,
				r->live[r->nlive].n, r->live[r->nlive].owner);
		pmm_free_pages(r->live[r->nlive].phys, r->live[r->nlive].n,
			r->live[r->nlive].owner);
		r->frees++;
	}
	r->frames = 0;
	run_check(r);
}
