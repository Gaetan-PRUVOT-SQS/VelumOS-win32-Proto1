#include "velum/err.h"
#include "velum/vmem.h"
#include "velum/vstart.h"

int	relro_lock(uint64_t start, uint64_t end)
{
	if (start > end || start > UINT64_MAX - (PAGE_SIZE - 1))
		return (E_INVAL);
	start = align_up(start, PAGE_SIZE);
	end = align_down(end, PAGE_SIZE);
	if (end <= start)
		return (0);
	return (v_vprotect(start, end - start, PROT_R));
}
