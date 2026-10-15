#include "pmm_int.h"

uint64_t	pmm_range_end(const t_memrange *r)
{
	if (r->length > UINT64_MAX - r->base)
		return (UINT64_MAX);
	return (r->base + r->length);
}

int	pmm_is_pool_type(uint32_t type)
{
	return (type == MEM_USABLE || type == MEM_BOOT_RECLAIM);
}

int	pmm_piece(const t_memrange *r, uint64_t floor, t_pmm_span *out)
{
	if (r->base > UINT64_MAX - (PAGE_SIZE - 1))
		return (0);
	out->lo = max_u64(align_up(r->base, PAGE_SIZE), floor);
	out->hi = align_down(pmm_range_end(r), PAGE_SIZE);
	return (out->hi > out->lo);
}

static int	overlaps(const t_memrange *a, const t_memrange *b)
{
	if (!a->length || !b->length)
		return (0);
	return (a->base < pmm_range_end(b) && b->base < pmm_range_end(a));
}

int	pmm_ranges_sane(const t_bootinfo *bi)
{
	uint32_t	i;
	uint32_t	j;

	i = 0;
	while (i < bi->nranges)
	{
		j = 0;
		while (pmm_is_pool_type(bi->ranges[i].type) && j < bi->nranges)
		{
			if (i != j && overlaps(&bi->ranges[i], &bi->ranges[j]))
				return (0);
			j++;
		}
		i++;
	}
	return (1);
}
