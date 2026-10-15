#include "velum/err.h"
#include "fake.h"
#include "model.h"

uint32_t	model_rand(t_model *m)
{
	m->rng ^= m->rng << 13;
	m->rng ^= m->rng >> 7;
	m->rng ^= m->rng << 17;
	return ((uint32_t)(m->rng >> 11));
}

int	model_expect(const t_model *m, uint32_t op, uint32_t a, uint32_t n)
{
	uint32_t	i;
	int			any;
	int			all;

	any = 0;
	all = 1;
	i = a;
	while (i < a + n)
	{
		any |= m->mapped[i];
		all &= m->mapped[i];
		i++;
	}
	if (op == 0 && any)
		return (E_EXIST);
	if (op == 2 && !all)
		return (E_NOENT);
	return (0);
}

int	model_call(uint32_t op, uint32_t a, uint32_t n, uint32_t prot)
{
	uintptr_t	va;

	va = UVA + (uintptr_t)a * 4096;
	if (op == 0)
		return (vmm_alloc(g_vmm.current, va, n * 4096, prot | VM_USER));
	if (op == 1)
		return (vmm_unmap(g_vmm.current, va, n * 4096));
	return (vmm_protect(g_vmm.current, va, n * 4096, prot));
}

void	model_update(t_model *m, uint32_t op, uint32_t a, uint32_t n)
{
	uint32_t	i;

	i = a;
	while (i < a + n)
	{
		m->mapped[i] = (op != 1);
		if (op != 1)
			m->prot[i] = (uint8_t)m->next_prot;
		i++;
	}
}

void	model_compare(t_model *m)
{
	uint32_t	i;
	uint64_t	pte;

	i = 0;
	while (i < MODEL_PAGES)
	{
		pte = fake_pte(g_vmm.current, UVA + (uintptr_t)i * 4096);
		if ((pte != 0) != (m->mapped[i] != 0))
			m->mismatches++;
		else if (pte && ((pte & PTE_W) != 0) != ((m->prot[i] & VM_W) != 0))
			m->mismatches++;
		i++;
	}
}
