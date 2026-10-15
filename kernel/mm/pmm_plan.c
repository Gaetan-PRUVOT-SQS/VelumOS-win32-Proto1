#include "velum/err.h"
#include "pmm_int.h"

static uint64_t	span_frames(const t_bootinfo *bi)
{
	uint32_t	i;
	uint64_t	top;
	t_pmm_span	s;

	i = 0;
	top = 0;
	while (i < bi->nranges)
	{
		if (pmm_is_pool_type(bi->ranges[i].type)
			&& pmm_piece(&bi->ranges[i], PAGE_SIZE, &s))
			top = max_u64(top, s.hi);
		i++;
	}
	return (top >> PAGE_SHIFT);
}

static uint64_t	meta_spot(const t_bootinfo *bi, uint64_t pages)
{
	uint32_t	i;
	uint64_t	best_hi;
	uint64_t	best_len;
	t_pmm_span	s;

	i = 0;
	best_hi = 0;
	best_len = 0;
	while (i < bi->nranges)
	{
		if (bi->ranges[i].type == MEM_USABLE
			&& pmm_piece(&bi->ranges[i], PMM_LOW_END, &s)
			&& s.hi - s.lo >= best_len)
		{
			best_len = s.hi - s.lo;
			best_hi = s.hi;
		}
		i++;
	}
	if (!best_len || best_len < (pages << PAGE_SHIFT))
		return (0);
	return (best_hi - (pages << PAGE_SHIFT));
}

uint64_t	pmm_meta_pages(uint64_t span)
{
	uint64_t	bytes;

	bytes = (span + 63) / 64 * 8 + span;
	return (align_up(bytes, PAGE_SIZE) >> PAGE_SHIFT);
}

int	pmm_plan_build(const t_bootinfo *bi, t_pmm_plan *plan)
{
	if (bi->nranges > BOOT_MAX_RANGES || !pmm_ranges_sane(bi))
		return (E_PROTO);
	plan->span = span_frames(bi);
	if (!plan->span)
		return (E_NOMEM);
	if (plan->span > PMM_MAX_FRAMES)
		return (E_PROTO);
	plan->meta_pages = pmm_meta_pages(plan->span);
	plan->meta_phys = meta_spot(bi, plan->meta_pages);
	if (!plan->meta_phys)
		return (E_NOMEM);
	return (E_OK);
}
