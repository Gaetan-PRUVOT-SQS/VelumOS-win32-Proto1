#include "velum/err.h"
#include "velum/vmem.h"
#include "velum/vobj.h"
#include "ws_sys.h"

int64_t	ws_sys_section(uint64_t size)
{
	return (v_section_create(size, PROT_R | PROT_W));
}

void	*ws_sys_map(t_handle section, uint64_t len)
{
	int64_t	va;

	va = v_section_map(section, 0, len, PROT_R);
	if (va <= 0)
		return (NULL);
	return ((void *)(uintptr_t)va);
}

void	ws_sys_unmap(void *va, uint64_t len)
{
	if (va != NULL)
		v_vfree((uint64_t)(uintptr_t)va, align_up(len, PAGE_SIZE));
}

void	*ws_sys_alloc(uint64_t size)
{
	int64_t	va;

	if (size == 0)
		return (NULL);
	va = v_valloc(0, align_up(size, PAGE_SIZE), PROT_R | PROT_W);
	if (va <= 0)
		return (NULL);
	return ((void *)(uintptr_t)va);
}

void	ws_sys_free(void *p, uint64_t size)
{
	if (p != NULL)
		v_vfree((uint64_t)(uintptr_t)p, align_up(size, PAGE_SIZE));
}
