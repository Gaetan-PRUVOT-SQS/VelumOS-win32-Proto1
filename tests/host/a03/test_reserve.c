#include "velum/err.h"
#include "harness.h"
#include "fake.h"

#define GUARD 0x500000ull
#define RW 0x0b

static void	reserve_sans_page_ni_frame(void)
{
	t_aspace	*as;
	uint64_t	kernel;
	uint64_t	used;

	fake_reset();
	as = fake_user();
	vmm_alloc(as, UVA, 4096, RW);
	kernel = g_fake.live[PMM_KERNEL];
	used = vmm_pages_used(as);
	h_eq_u64("regions avant", vmm_region_count(as), 1);
	h_eq_i64("reserve", vmm_reserve(as, GUARD, 4096), 0);
	h_eq_u64("une region de plus", vmm_region_count(as), 2);
	h_eq_u64("aucune frame utilisateur", g_fake.live[PMM_USER], 1);
	h_eq_u64("aucune frame noyau", g_fake.live[PMM_KERNEL], kernel);
	h_eq_u64("pages comptees inchangees", vmm_pages_used(as), used);
	h_true(!fake_pte(as, GUARD), "aucune entree de table");
	h_eq_i64("lecture user refusee", vmm_user_check(as, GUARD, 1, false),
		E_FAULT);
	h_eq_i64("non aligne", vmm_reserve(as, GUARD + 1, 4096), E_INVAL);
	h_eq_i64("espace nul", vmm_reserve(NULL, GUARD, 4096), E_INVAL);
	h_eq_i64("hors zone user", vmm_reserve(as, USER_TOP, 4096), E_INVAL);
	vmm_aspace_destroy(as);
	fake_clean("faux pmm propre");
}

static void	table_reservee_origine_noyau(void)
{
	t_aspace	*as;
	t_vmreq		rq;

	fake_reset();
	as = fake_user();
	vmm_reserve(as, GUARD, 4096);
	rq.va = GUARD;
	rq.pa = FAKE_BASE;
	rq.len = 4096;
	rq.flags = VM_R | VM_USER;
	h_eq_u64("recherche : jamais la reserve", vmm_find_free(as, 4096, GUARD,
			GUARD + 4096), 0);
	h_eq_u64("recherche : page suivante", vmm_find_free(as, 4096, GUARD,
			USER_TOP), GUARD + 4096);
	h_eq_i64("alloc fixe", vmm_alloc(as, GUARD, 4096, RW), E_EXIST);
	h_eq_i64("map fixe", vmm_map(as, &rq), E_EXIST);
	h_eq_i64("reserve en double", vmm_reserve(as, GUARD, 4096), E_EXIST);
	h_eq_i64("unmap generique refuse", vmm_unmap(as, GUARD, 4096), E_ACCES);
	h_eq_i64("unreserve bornes fausses", vmm_unreserve(as, GUARD, 8192),
		E_NOENT);
	h_eq_i64("unreserve dedie", vmm_unreserve(as, GUARD, 4096), 0);
	h_eq_u64("plus de region", vmm_region_count(as), 0);
	h_eq_i64("alloc apres retrait", vmm_alloc(as, GUARD, 4096, RW), 0);
	vmm_aspace_destroy(as);
	fake_clean("faux pmm propre");
}

static void	table_origine_utilisateur(void)
{
	t_aspace	*as;

	fake_reset();
	as = fake_user();
	vmm_reserve(as, GUARD, 4096);
	vmm_alloc(as, GUARD + 4096, 8192, RW);
	h_eq_i64("unmap user reserve", vmm_unmap(as, GUARD, 4096), E_ACCES);
	h_eq_i64("unmap user a cheval", vmm_unmap(as, GUARD, 3 * 4096),
		E_ACCES);
	h_eq_i64("unmap user tout l'espace", vmm_unmap(as, USER_MIN,
			USER_TOP - USER_MIN), E_ACCES);
	h_eq_u64("rien retire", vmm_region_count(as), 2);
	h_true(fake_pte(as, GUARD + 4096) != 0, "voisine intacte");
	h_eq_i64("unmap user normal", vmm_unmap(as, GUARD + 4096, 4096), 0);
	h_eq_i64("unmap user trou", vmm_unmap(as, UVA, 4096), 0);
	h_eq_i64("unmap user non aligne", vmm_unmap(as, GUARD + 1, 4096),
		E_INVAL);
	h_eq_i64("unmap user espace nul", vmm_unmap(NULL, GUARD, 4096),
		E_INVAL);
	h_eq_i64("protect voisine", vmm_protect(as, GUARD + 8192, 4096, VM_R), 0);
	h_eq_i64("protect a cheval", vmm_protect(as, GUARD, 3 * 4096, VM_R),
		E_NOENT);
	h_eq_i64("reserve toujours la", vmm_alloc(as, GUARD, 4096, RW), E_EXIST);
	vmm_aspace_destroy(as);
	fake_clean("faux pmm propre");
}

static void	reserve_echec_du_descripteur(void)
{
	t_aspace	*as;
	uint64_t	va;
	uint64_t	n;

	fake_reset();
	as = fake_user();
	va = GUARD;
	while (as->nfree > 0)
	{
		vmm_reserve(as, va, 4096);
		va += 8192;
	}
	n = vmm_region_count(as);
	pmm_fail_after(0);
	h_eq_i64("pool vide et pmm refuse", vmm_reserve(as, va, 4096), E_NOMEM);
	pmm_fail_after(-1);
	h_eq_u64("aucune region ajoutee", vmm_region_count(as), n);
	h_eq_u64("place toujours libre", vmm_find_free(as, 4096, va, va + 4096),
		va);
	h_eq_i64("reussit ensuite", vmm_reserve(as, va, 4096), 0);
	h_eq_u64("une region de plus", vmm_region_count(as), n + 1);
	vmm_aspace_destroy(as);
	fake_clean("faux pmm propre");
}

int	main(void)
{
	h_begin("a03/reserve");
	h_run("reserve/sans-page-ni-frame", reserve_sans_page_ni_frame);
	h_run("reserve/table-origine-noyau", table_reservee_origine_noyau);
	h_run("reserve/table-origine-utilisateur", table_origine_utilisateur);
	h_run("reserve/echec-du-descripteur", reserve_echec_du_descripteur);
	return (h_end());
}
