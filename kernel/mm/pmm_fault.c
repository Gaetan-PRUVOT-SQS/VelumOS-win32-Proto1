#include "velum/klog.h"
#include "velum/panic.h"
#include "pmm_int.h"

const char	*pmm_fault_name(int fault)
{
	if (fault == PMM_FAULT_ALIGN)
		return ("pmm: adresse non alignée");
	if (fault == PMM_FAULT_RANGE)
		return ("pmm: adresse hors de la mémoire gérée");
	if (fault == PMM_FAULT_DOUBLE)
		return ("pmm: double libération");
	return ("pmm: mauvais propriétaire");
}

static int	holder_of(uint64_t phys)
{
	if ((phys >> PAGE_SHIFT) >= g_pmm.span)
		return (-1);
	return (g_pmm.owner[phys >> PAGE_SHIFT]);
}

void	pmm_fault_report(int fault, uint64_t phys, uint64_t n, void *from)
{
	klog_err("%s phys=%#llx n=%llu détenteur=%d appelant=%p",
		pmm_fault_name(fault), phys, n, holder_of(phys), from);
	kassert_check(0, pmm_fault_name(fault));
}
