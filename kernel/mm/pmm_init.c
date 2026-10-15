#include "velum/err.h"
#include "velum/klog.h"
#include "pmm_int.h"

static void	count_pool(void)
{
	g_pmm.stats.free_pages = pmm_bits_count_free();
	g_pmm.stats.total_pages = g_pmm.stats.free_pages;
	g_pmm.stats.reserved_pages = g_pmm.span - g_pmm.stats.free_pages;
	g_pmm.hint = PMM_LOW_FRAMES;
}

static const char	*range_label(uint32_t type)
{
	if (type == MEM_USABLE)
		return ("libre");
	return ("reprise plus tard");
}

static void	journal_ranges(const t_bootinfo *bi)
{
	uint32_t	i;
	t_pmm_span	s;

	i = 0;
	while (i < bi->nranges)
	{
		if (pmm_is_pool_type(bi->ranges[i].type)
			&& pmm_piece(&bi->ranges[i], PAGE_SIZE, &s))
			klog_info("pmm: plage %#llx-%#llx %s", s.lo, s.hi,
				range_label(bi->ranges[i].type));
		i++;
	}
}

static void	journal_totals(void)
{
	const t_pmm_stats	*s;

	s = &g_pmm.stats;
	klog_info("pmm: total=%llu libre=%llu meta=%llu reserve=%llu Kio",
		s->total_pages * 4, s->free_pages * 4, g_pmm.meta_pages * 4,
		s->reserved_pages * 4);
}

int	pmm_boot_init(void)
{
	const t_bootinfo	*bi;
	t_pmm_plan			plan;
	int					rc;

	if (g_pmm.bits)
		return (E_BUSY);
	bi = boot_info();
	rc = pmm_plan_build(bi, &plan);
	if (rc < 0)
		return (rc);
	pmm_build_tables(bi, &plan);
	count_pool();
	if (!g_pmm.stats.free_pages)
	{
		pmm_reset();
		return (E_NOMEM);
	}
	journal_totals();
	journal_ranges(bi);
	return (E_OK);
}
