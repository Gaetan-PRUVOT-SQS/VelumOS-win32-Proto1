#include "pmm_int.h"

static int	st_budget(void)
{
	uint64_t	first;

	pmm_fail_after(1);
	first = pmm_alloc(PMM_KERNEL);
	if (!first || pmm_alloc(PMM_KERNEL) || pmm_alloc_zero(PMM_KERNEL))
		return (1);
	if (pmm_alloc_pages(PMM_KERNEL, 2, 1, 0))
		return (2);
	pmm_fail_after(-1);
	pmm_free(first, PMM_KERNEL);
	return (0);
}

int	pmm_st_fail(void)
{
	t_pmm_stats	before;
	t_pmm_stats	now;
	int			rc;

	pmm_get_stats(&before);
	rc = st_budget();
	if (rc)
		return (rc);
	pmm_get_stats(&now);
	if (now.fail_calls != before.fail_calls + 3)
		return (3);
	if (now.free_pages != before.free_pages)
		return (4);
	return (0);
}
