#include "a02_fake.h"

static uint64_t	pick_size(void)
{
	uint64_t	r;

	r = fake_below(100);
	if (r < 60)
		return (1);
	if (r < 90)
		return (2 + fake_below(7));
	return (9 + fake_below(62));
}

uint64_t	fake_random_fill(t_farg *blk, uint64_t cap)
{
	uint64_t	n;
	uint64_t	al;
	t_pmm_owner	o;

	n = 0;
	while (n < cap)
	{
		al = 1ull << fake_below(4);
		o = (t_pmm_owner)(PMM_KERNEL + fake_below(PMM_OWNERS - 1));
		blk[n].n = pick_size();
		blk[n].owner = o;
		blk[n].phys = pmm_alloc_pages(o, blk[n].n, al, 0);
		if (!blk[n].phys)
			break ;
		n++;
	}
	return (n);
}

void	fake_shuffle_free(t_farg *blk, uint64_t n)
{
	uint64_t	i;
	uint64_t	j;
	t_farg		tmp;

	i = n;
	while (i > 1)
	{
		j = fake_below(i--);
		tmp = blk[i];
		blk[i] = blk[j];
		blk[j] = tmp;
	}
	i = 0;
	while (i < n)
	{
		pmm_free_pages(blk[i].phys, blk[i].n, blk[i].owner);
		i++;
	}
}
