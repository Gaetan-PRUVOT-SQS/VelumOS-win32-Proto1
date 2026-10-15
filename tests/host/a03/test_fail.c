#include "velum/err.h"
#include "harness.h"
#include "fake.h"

static int	alloc_au_rang(int64_t k)
{
	t_aspace	*as;
	uint64_t	tables;
	int			rc;

	fake_reset();
	as = fake_user();
	vmm_pool_reserve(as, 2);
	tables = g_fake.live[PMM_PAGETABLE];
	pmm_fail_after(k);
	rc = vmm_alloc(as, 0x3fe000, 4 * 4096, VM_R | VM_W | VM_USER);
	pmm_fail_after(-1);
	if (rc == 0)
		return (0);
	h_eq_i64("E_NOMEM", rc, E_NOMEM);
	h_eq_u64("aucune frame user", g_fake.live[PMM_USER], 0);
	h_eq_u64("tables revenues", g_fake.live[PMM_PAGETABLE], tables);
	h_true(!fake_pte(as, 0x3fe000) && !fake_pte(as, 0x400000), "rien mappe");
	h_eq_u64("region libre", vmm_find_free(as, 4 * 4096, 0x3fe000, USER_TOP),
		0x3fe000);
	vmm_aspace_destroy(as);
	fake_clean("faux pmm propre");
	return (1);
}

static void	echec_alloc_chaque_rang(void)
{
	int64_t	k;

	k = 0;
	while (alloc_au_rang(k))
		k++;
	h_eq_i64("rangs essayes (4 frames + 4 tables)", k, 8);
	fake_clean("faux pmm propre");
}

static int	map_au_rang(int64_t k)
{
	t_aspace	*as;
	t_vmreq		rq;
	int			rc;

	fake_reset();
	as = fake_user();
	vmm_pool_reserve(as, 2);
	rq.va = 0x3ff000;
	rq.pa = FAKE_BASE;
	rq.len = 2 * 4096;
	rq.flags = VM_R | VM_USER;
	pmm_fail_after(k);
	rc = vmm_map(as, &rq);
	pmm_fail_after(-1);
	if (rc == 0)
		return (0);
	h_eq_i64("E_NOMEM", rc, E_NOMEM);
	h_eq_u64("tables revenues", as->tables, 1);
	h_true(!fake_pte(as, 0x3ff000), "rien mappe");
	vmm_aspace_destroy(as);
	fake_clean("faux pmm propre");
	return (1);
}

static void	echec_map_chaque_rang(void)
{
	int64_t	k;

	k = 0;
	while (map_au_rang(k))
		k++;
	h_eq_i64("rangs essayes (4 tables)", k, 4);
}

int	main(void)
{
	h_begin("a03/fail");
	h_run("echec_alloc_chaque_rang", echec_alloc_chaque_rang);
	h_run("echec_map_chaque_rang", echec_map_chaque_rang);
	return (h_end());
}
