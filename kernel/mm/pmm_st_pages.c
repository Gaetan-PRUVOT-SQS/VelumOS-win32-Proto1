#include "velum/libk.h"
#include "pmm_int.h"

static int	st_contiguous(uint64_t phys, uint64_t pages)
{
	uint64_t	i;

	i = 0;
	while (i < pages)
	{
		if (g_pmm.owner[(phys >> PAGE_SHIFT) + i] != PMM_DMA)
			return (0);
		i++;
	}
	memset(pmm_virt(phys), 0x5a, pages << PAGE_SHIFT);
	return (pmm_st_frame_is(phys, pages, 0x5a));
}

static int	st_aligned(uint64_t pages, uint64_t align)
{
	t_pmm_stats	before;
	t_pmm_stats	now;
	uint64_t	phys;

	pmm_get_stats(&before);
	phys = pmm_alloc_pages(PMM_DMA, pages, align, 0);
	if (!phys || phys % (align * PAGE_SIZE))
		return (1);
	if (!st_contiguous(phys, pages))
		return (2);
	pmm_get_stats(&now);
	if (now.owned[PMM_DMA] != before.owned[PMM_DMA] + pages)
		return (3);
	pmm_free_pages(phys, pages, PMM_DMA);
	pmm_get_stats(&now);
	if (now.free_pages != before.free_pages)
		return (4);
	return (0);
}

int	pmm_st_pages(void)
{
	if (st_aligned(8, 4))
		return (1);
	if (st_aligned(512, 512))
		return (2);
	if (st_aligned(3, 1))
		return (3);
	return (0);
}

static int	low_free_exists(void)
{
	return (pmm_bits_next(1, PMM_LOW_FRAMES, 0) < PMM_LOW_FRAMES);
}

int	pmm_st_low(void)
{
	uint64_t	phys;

	phys = pmm_alloc_pages(PMM_DRIVER, 1, 1, PMM_LOW_END);
	if (!phys)
		return (low_free_exists());
	if (phys >= PMM_LOW_END || g_pmm.owner[phys >> PAGE_SHIFT] != PMM_DRIVER)
		return (2);
	pmm_free(phys, PMM_DRIVER);
	return (0);
}
