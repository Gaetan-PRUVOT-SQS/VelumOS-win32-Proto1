#include "velum/err.h"
#include "velum/klog.h"
#include "velum/libk.h"
#include "pmm_int.h"

static int	in_reclaim(const t_pmm_span *s)
{
	const t_bootinfo	*bi;
	t_pmm_span			r;
	uint32_t			i;

	bi = boot_info();
	i = 0;
	while (i < bi->nranges && i < BOOT_MAX_RANGES)
	{
		if (bi->ranges[i].type == MEM_BOOT_RECLAIM
			&& pmm_piece(&bi->ranges[i], PAGE_SIZE, &r)
			&& r.lo <= s->lo && s->hi <= r.hi)
			return (1);
		i++;
	}
	return (0);
}

static int	region_commit(uint64_t from, uint64_t to)
{
	uint64_t	i;

	i = from;
	while (i < to)
	{
		if (g_pmm.owner[i] != PMM_MARK_RESERVED)
			return (E_EXIST);
		i++;
	}
	pmm_bits_fill(from, to, 0);
	memset(g_pmm.owner + from, PMM_FREE, to - from);
	g_pmm.stats.total_pages += to - from;
	g_pmm.stats.free_pages += to - from;
	g_pmm.stats.reserved_pages -= to - from;
	return ((int)min_u64(to - from, INT32_MAX));
}

static int	region_span(uint64_t base, uint64_t length, t_pmm_span *s)
{
	t_memrange	r;

	if (!g_pmm.bits || !length || length > UINT64_MAX - base)
		return (E_INVAL);
	r.base = base;
	r.length = length;
	r.type = MEM_BOOT_RECLAIM;
	r.reserved = 0;
	if (!pmm_piece(&r, PAGE_SIZE, s))
		return (0);
	if (!in_reclaim(s))
		return (E_PERM);
	if ((s->hi >> PAGE_SHIFT) > g_pmm.span)
		return (E_RANGE);
	return (1);
}

int	pmm_add_region(uint64_t base, uint64_t length)
{
	t_pmm_span	s;
	uint64_t	flags;
	int			rc;

	rc = region_span(base, length, &s);
	if (rc <= 0)
		return (rc);
	flags = pmm_lock();
	rc = region_commit(s.lo >> PAGE_SHIFT, s.hi >> PAGE_SHIFT);
	pmm_unlock(flags);
	if (rc > 0)
		klog_info("pmm: +%d pages reprises du chargeur", rc);
	return (rc);
}
