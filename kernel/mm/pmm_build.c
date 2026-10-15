#include "velum/libk.h"
#include "pmm_int.h"

static void	attach_meta(const t_pmm_plan *plan)
{
	g_pmm.span = plan->span;
	g_pmm.words = (plan->span + 63) / 64;
	g_pmm.meta_phys = plan->meta_phys;
	g_pmm.meta_pages = plan->meta_pages;
	g_pmm.bits = pmm_virt(plan->meta_phys);
	g_pmm.owner = (uint8_t *)(g_pmm.bits + g_pmm.words);
}

static void	reserve_all(void)
{
	memset(g_pmm.bits, 0xff, g_pmm.words * sizeof(uint64_t));
	memset(g_pmm.owner, PMM_MARK_RESERVED, g_pmm.span);
}

static void	mark_pool(const t_bootinfo *bi)
{
	uint32_t	i;
	t_pmm_span	s;

	i = 0;
	while (i < bi->nranges)
	{
		if (bi->ranges[i].type == MEM_USABLE
			&& pmm_piece(&bi->ranges[i], PAGE_SIZE, &s))
		{
			pmm_bits_fill(s.lo >> PAGE_SHIFT, s.hi >> PAGE_SHIFT, 0);
			memset(g_pmm.owner + (s.lo >> PAGE_SHIFT), PMM_FREE,
				(s.hi - s.lo) >> PAGE_SHIFT);
		}
		i++;
	}
}

static void	mark_meta(void)
{
	uint64_t	first;

	first = g_pmm.meta_phys >> PAGE_SHIFT;
	pmm_bits_fill(first, first + g_pmm.meta_pages, 1);
	memset(g_pmm.owner + first, PMM_MARK_META, g_pmm.meta_pages);
}

void	pmm_build_tables(const t_bootinfo *bi, const t_pmm_plan *plan)
{
	g_pmm.hhdm = bi->hhdm;
	attach_meta(plan);
	reserve_all();
	mark_pool(bi);
	mark_meta();
}
