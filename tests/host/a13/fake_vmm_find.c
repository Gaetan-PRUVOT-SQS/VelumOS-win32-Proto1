#include "fakes.h"
#include "velum/vmm.h"

uintptr_t	vmm_find_free(t_aspace *as, size_t len, uintptr_t lo, uintptr_t hi)
{
	uintptr_t	c;
	t_fmap		*m;
	uint32_t	call;

	call = g_fvmm.find_calls++;
	if (call < 32 && ((g_fvmm.fail_find_mask >> call) & 1))
		return (0);
	c = (lo + 4095) & ~(uintptr_t)4095;
	m = fake_vmm_overlap(as, c, len);
	while (m && c + len <= hi)
	{
		c = (m->va + m->len + 4095) & ~(uintptr_t)4095;
		m = fake_vmm_overlap(as, c, len);
	}
	if (c + len > hi)
		return (0);
	return (c);
}
