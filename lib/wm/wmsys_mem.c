#include "velum/vmem.h"
#include "velum/vobj.h"
#include "wmc_int.h"

void	*wmsys_map(t_handle section, uint64_t len)
{
	int64_t	va;

	va = v_section_map(section, 0, len, PROT_R | PROT_W);
	if (va <= 0)
		return (NULL);
	return ((void *)(uintptr_t)va);
}

void	wmsys_unmap(void *va, uint64_t len)
{
	if (va != NULL)
		v_vfree((uint64_t)(uintptr_t)va, align_up(len, PAGE_SIZE));
}

void	wmsys_close(t_handle h)
{
	if (h != 0)
		v_close(h);
}
