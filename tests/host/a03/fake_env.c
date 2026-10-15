#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "harness.h"
#include "fake.h"

t_fake	g_fake;

void	kassert_check(int cond, const char *msg)
{
	if (cond)
		return ;
	g_fake.errors++;
	fprintf(stderr, "kassert: %s\n", msg);
}

static void	fake_kernel(void)
{
	t_aspace	*kas;
	uint32_t	i;

	kas = &g_vmm.kas;
	kas->kernel = true;
	kas->lo = KHEAP_BASE;
	kas->hi = KSTACK_BASE + WIN_SIZE;
	kas->pml4 = pmm_alloc_zero(PMM_PAGETABLE);
	kas->tables = 1;
	i = 0;
	while (i < 6)
	{
		vmm_entry(kas, KHEAP_BASE + (i >> 1) * WIN_STRIDE
			+ (i & 1) * PML4_SPAN, PT_PDPT, true);
		i++;
	}
	g_vmm.current = kas;
	g_vmm.ready = true;
}

void	fake_reset(void)
{
	uint8_t	*arena;

	arena = g_fake.arena;
	if (!arena)
		arena = aligned_alloc(4096, FAKE_FRAMES * 4096ull);
	if (!arena)
		abort();
	memset(&g_fake, 0, sizeof(g_fake));
	g_fake.arena = arena;
	g_fake.fail_after = -1;
	memset(&g_vmm, 0, sizeof(g_vmm));
	g_vmm.nx = PTE_NX;
	g_vmm.uc_bits = PTE_PWT | PTE_PCD;
	g_vmm.wc_bits = PTE_PAT4K;
	g_vmm.hspan[0].base = FAKE_BASE;
	g_vmm.hspan[0].end = FAKE_BASE + FAKE_FRAMES * 4096ull;
	g_vmm.hspan[0].bits = PTE_P | PTE_W;
	g_vmm.nspan = 1;
	fake_kernel();
}

t_aspace	*fake_user(void)
{
	t_aspace	*as;

	as = vmm_aspace_create();
	h_true(as != NULL, "creation d'espace");
	if (!as)
		abort();
	vmm_switch(as);
	return (as);
}

void	fake_clean(const char *what)
{
	h_true(g_fake.errors == 0, what);
	if (g_fake.errors)
		fprintf(stderr, "  %d erreur(s) du faux pmm\n", g_fake.errors);
}
