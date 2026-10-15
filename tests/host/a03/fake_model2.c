#include "fake.h"
#include "model.h"

void	model_fill(t_aspace *as, int reverse)
{
	uint32_t	i;
	uint32_t	page;

	i = 0;
	while (i < 32)
	{
		page = i;
		if (reverse)
			page = 31 - i;
		vmm_alloc(as, UVA + page * 2 * 4096ull, 4096, VM_R | VM_W | VM_USER);
		i++;
	}
}

int	model_diff(t_aspace *a, t_aspace *b)
{
	uint32_t	i;
	int			diff;

	diff = 0;
	i = 0;
	while (i < 64)
	{
		diff += (fake_pte(a, UVA + i * 4096ull) & ~PTE_ADDR)
			!= (fake_pte(b, UVA + i * 4096ull) & ~PTE_ADDR);
		i++;
	}
	return (diff);
}
