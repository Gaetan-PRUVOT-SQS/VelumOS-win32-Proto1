#include "velum/boot.h"
#include "velum/klog.h"
#include "velum/libk.h"
#include "pmm_int.h"

static int	step(const char *name, int rc)
{
	if (rc)
		klog_err("pmm: autotest %s en échec (code %d)", name, rc);
	return (rc);
}

void	pmm_st_violation(void)
{
	const char	*mode;
	uint64_t	phys;

	mode = boot_cmdline_get("pmm_fault");
	if (!PMM_DEBUG || !mode)
		return ;
	phys = pmm_alloc(PMM_KERNEL);
	if (!strcmp(mode, "owner"))
		pmm_free(phys, PMM_USER);
	pmm_free(phys, PMM_KERNEL);
	if (!strcmp(mode, "align"))
		pmm_free(phys + 1, PMM_KERNEL);
	if (!strcmp(mode, "range"))
		pmm_free(g_pmm.span << PAGE_SHIFT, PMM_KERNEL);
	if (!strcmp(mode, "double"))
		pmm_free(phys, PMM_KERNEL);
}

int	pmm_selftest(void)
{
	int	rc;

	rc = step("alloc et libération", pmm_st_single());
	if (!rc)
		rc = step("pages contiguës", pmm_st_pages());
	if (!rc)
		rc = step("zone basse", pmm_st_low());
	if (!rc)
		rc = step("injection d'échecs", pmm_st_fail());
	if (!rc)
		rc = step("invariants", pmm_check());
	if (!rc)
		pmm_st_violation();
	return (rc);
}
