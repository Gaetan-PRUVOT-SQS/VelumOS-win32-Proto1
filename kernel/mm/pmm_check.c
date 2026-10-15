#include "velum/libk.h"
#include "pmm_int.h"

static int	slot_of(uint8_t owner)
{
	if (owner < PMM_OWNERS)
		return (owner);
	if (owner == PMM_MARK_RESERVED)
		return (PMM_OWNERS);
	if (owner == PMM_MARK_META)
		return (PMM_OWNERS + 1);
	return (-1);
}

static int	tally_frames(uint64_t *tally)
{
	uint64_t	f;
	int			slot;
	int			bad;

	f = 0;
	bad = 0;
	memset(tally, 0, (PMM_OWNERS + 2) * sizeof(uint64_t));
	while (f < g_pmm.span)
	{
		slot = slot_of(g_pmm.owner[f]);
		if (slot < 0)
			bad++;
		else
			tally[slot]++;
		if (pmm_bit_used(f) != (g_pmm.owner[f] != PMM_FREE))
			bad++;
		f++;
	}
	return (bad);
}

static int	compare_stats(const uint64_t *tally)
{
	const t_pmm_stats	*s;
	uint64_t			sum;
	int					o;
	int					bad;

	s = &g_pmm.stats;
	bad = (tally[PMM_FREE] != s->free_pages);
	sum = s->free_pages;
	o = PMM_KERNEL;
	while (o < PMM_OWNERS)
	{
		bad += (tally[o] != s->owned[o]);
		sum += s->owned[o];
		o++;
	}
	bad += (tally[PMM_OWNERS] + tally[PMM_OWNERS + 1] != s->reserved_pages);
	bad += (sum != s->total_pages);
	bad += (g_pmm.span != s->total_pages + s->reserved_pages);
	return (bad);
}

static int	padding_bad(void)
{
	uint64_t	end;

	end = g_pmm.words * 64;
	return (pmm_bits_next(g_pmm.span, end, 0) != end);
}

int	pmm_check(void)
{
	uint64_t	tally[PMM_OWNERS + 2];
	uint64_t	flags;
	int			bad;

	flags = pmm_lock();
	bad = tally_frames(tally);
	bad += compare_stats(tally);
	bad += padding_bad();
	pmm_unlock(flags);
	return (bad);
}
