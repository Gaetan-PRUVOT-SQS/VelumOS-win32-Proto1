#include "sys_int.h"
#include "velum/vmem.h"

int64_t	v_valloc(uint64_t hint_va, uint64_t len, uint32_t prot)
{
	return (sys3(SYS_VALLOC, hint_va, len, prot));
}

int	v_vfree(uint64_t va, uint64_t len)
{
	return ((int)sys2(SYS_VFREE, va, len));
}

int	v_vprotect(uint64_t va, uint64_t len, uint32_t prot)
{
	return ((int)sys3(SYS_VPROTECT, va, len, prot));
}

int	v_vquery(uint64_t va, t_vquery *out)
{
	return ((int)sys2(SYS_VQUERY, va, sys_ptr(out)));
}
