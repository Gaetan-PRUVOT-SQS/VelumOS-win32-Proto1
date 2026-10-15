#include "velum/libk.h"
#include "pmm_int.h"

int	pmm_st_frame_is(uint64_t phys, uint64_t pages, uint8_t byte)
{
	const uint8_t	*p;
	uint64_t		i;

	p = pmm_virt(phys);
	i = 0;
	while (i < (pages << PAGE_SHIFT))
	{
		if (p[i] != byte)
			return (0);
		i++;
	}
	return (1);
}

static int	st_counters(uint64_t phys, const t_pmm_stats *before)
{
	t_pmm_stats	now;

	pmm_get_stats(&now);
	if (now.free_pages != before->free_pages - 1)
		return (1);
	if (now.owned[PMM_KERNEL] != before->owned[PMM_KERNEL] + 1)
		return (2);
	if (g_pmm.owner[phys >> PAGE_SHIFT] != PMM_KERNEL)
		return (3);
	return (0);
}

static int	st_zero(uint64_t phys)
{
	uint64_t	again;

	g_pmm.hint = phys >> PAGE_SHIFT;
	again = pmm_alloc_zero(PMM_USER);
	if (again != phys)
		return (1);
	if (!pmm_st_frame_is(again, 1, 0))
		return (2);
	pmm_free(again, PMM_USER);
	return (0);
}

static int	st_free_back(uint64_t phys, const t_pmm_stats *before)
{
	t_pmm_stats	now;

	memset(pmm_virt(phys), 0xa5, PAGE_SIZE);
	pmm_free(phys, PMM_KERNEL);
	if (PMM_DEBUG && !pmm_st_frame_is(phys, 1, PMM_POISON))
		return (1);
	if (st_zero(phys))
		return (2);
	pmm_get_stats(&now);
	if (now.free_pages != before->free_pages)
		return (3);
	if (now.owned[PMM_KERNEL] != before->owned[PMM_KERNEL])
		return (4);
	return (0);
}

int	pmm_st_single(void)
{
	t_pmm_stats	before;
	uint64_t	phys;
	int			rc;

	pmm_get_stats(&before);
	phys = pmm_alloc(PMM_KERNEL);
	if (!phys || !is_aligned(phys, PAGE_SIZE) || phys < PMM_LOW_END)
		return (1);
	rc = st_counters(phys, &before);
	if (rc)
		return (10 + rc);
	rc = st_free_back(phys, &before);
	if (rc)
		return (20 + rc);
	return (0);
}
