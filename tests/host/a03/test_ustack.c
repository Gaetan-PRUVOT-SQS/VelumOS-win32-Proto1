#include "velum/err.h"
#include "harness.h"
#include "fake.h"

#define SPOT 0x600000ull
#define SLEN 0x4000ull
#define RW 0x0b

static void	pile_posee_d_un_bloc(void)
{
	t_aspace	*as;
	uintptr_t	va;

	fake_reset();
	as = fake_user();
	va = 0;
	h_eq_i64("place cherchee", vmm_stack_map(as, &va, SLEN), 0);
	h_true(va >= ASLR_LO && va < ASLR_HI, "dans la zone ASLR");
	h_eq_u64("garde et pile", vmm_region_count(as), 2);
	h_eq_u64("frames de la pile seule", g_fake.live[PMM_USER], 4);
	h_true(!fake_pte(as, va) && fake_pte(as, va + 4096), "garde sans page");
	va = SPOT;
	h_eq_i64("place fixe", vmm_stack_map(as, &va, SLEN), 0);
	h_eq_i64("place fixe prise", vmm_stack_map(as, &va, SLEN), E_EXIST);
	vmm_alloc(as, UVA + 4096, 4096, RW);
	va = UVA;
	h_eq_i64("pile sur un voisin", vmm_stack_map(as, &va, SLEN), E_EXIST);
	h_eq_i64("garde non posee", vmm_alloc(as, UVA, 4096, RW), 0);
	va = SPOT + 1;
	h_eq_i64("non aligne", vmm_stack_map(as, &va, SLEN), E_INVAL);
	h_eq_i64("taille nulle", vmm_stack_map(as, &va, 0), E_INVAL);
	h_eq_i64("espace nul", vmm_stack_map(NULL, &va, SLEN), E_INVAL);
	vmm_aspace_destroy(as);
	fake_clean("faux pmm propre");
}

static void	table_pile_tenue(void)
{
	t_aspace	*as;
	uintptr_t	va;

	fake_reset();
	as = fake_user();
	va = SPOT;
	vmm_stack_map(as, &va, SLEN);
	vmm_alloc(as, SPOT + 4096 + SLEN, 4096, RW);
	h_eq_i64("unmap page de pile", vmm_unmap(as, SPOT + 4096, 4096), E_ACCES);
	h_eq_i64("unmap garde et pile", vmm_unmap(as, SPOT, 4096 + SLEN),
		E_ACCES);
	h_eq_i64("unmap voisine et pile", vmm_unmap(as, SPOT + SLEN, 8192),
		E_ACCES);
	h_eq_i64("protect pile", vmm_protect(as, SPOT + 4096, 4096, VM_R),
		E_ACCES);
	h_eq_i64("protect garde", vmm_protect(as, SPOT, 4096, VM_R), E_NOENT);
	h_eq_i64("unreserve sur la pile", vmm_unreserve(as, SPOT + 4096, SLEN),
		E_NOENT);
	h_eq_u64("rien retire", vmm_region_count(as), 3);
	h_eq_u64("frames intactes", g_fake.live[PMM_USER], 5);
	h_eq_i64("voisine normale libre", vmm_unmap(as, SPOT + 4096 + SLEN, 4096),
		0);
	vmm_aspace_destroy(as);
	fake_clean("faux pmm propre");
}

static void	retrait_dedie_bornes_exactes(void)
{
	t_aspace	*as;
	uintptr_t	va;

	fake_reset();
	as = fake_user();
	va = SPOT;
	vmm_stack_map(as, &va, SLEN);
	vmm_reserve(as, UVA, 4096);
	vmm_alloc(as, UVA + 4096, SLEN, RW);
	h_eq_i64("pile trop courte", vmm_stack_unmap(as, SPOT, SLEN - 4096),
		E_NOENT);
	h_eq_i64("pile trop longue", vmm_stack_unmap(as, SPOT, SLEN + 4096),
		E_NOENT);
	h_eq_i64("base decalee", vmm_stack_unmap(as, SPOT + 4096, SLEN), E_NOENT);
	h_eq_i64("region normale", vmm_stack_unmap(as, UVA, SLEN), E_NOENT);
	h_eq_i64("espace nul", vmm_stack_unmap(NULL, SPOT, SLEN), E_INVAL);
	h_eq_u64("rien retire", vmm_region_count(as), 4);
	h_eq_i64("bornes exactes", vmm_stack_unmap(as, SPOT, SLEN), 0);
	h_eq_u64("garde et pile rendues", vmm_region_count(as), 2);
	h_eq_u64("frames rendues", g_fake.live[PMM_USER], 4);
	h_eq_i64("deja rendue", vmm_stack_unmap(as, SPOT, SLEN), E_NOENT);
	h_eq_u64("place libre", vmm_find_free(as, 4096 + SLEN, SPOT, USER_TOP),
		SPOT);
	vmm_aspace_destroy(as);
	fake_clean("faux pmm propre");
}

static void	echecs_sans_reste(void)
{
	t_aspace	*as;
	uintptr_t	va;
	uint64_t	n;

	fake_reset();
	as = fake_user();
	va = SPOT;
	vmm_stack_map(as, &va, SLEN);
	while (as->nfree != 2)
		vmm_reserve(as, va += 0x10000, 4096);
	n = vmm_region_count(as);
	va = 0;
	pmm_fail_after(0);
	h_eq_i64("deux descripteurs", vmm_stack_map(as, &va, SLEN), E_NOMEM);
	h_eq_u64("rien pose", vmm_region_count(as), n);
	while (as->nfree)
		vmm_reserve(as, (va += 0x10000) - 0x8000, 4096);
	h_eq_i64("pool vide", vmm_stack_map(as, &va, SLEN), E_NOMEM);
	h_eq_i64("pile rendue pool vide", vmm_stack_unmap(as, SPOT, SLEN), 0);
	h_eq_i64("reserve rendue pool vide", vmm_unreserve(as, SPOT + 0x10000,
			4096), 0);
	pmm_fail_after(-1);
	h_eq_u64("compteur revenu", vmm_region_count(as), n - 1);
	vmm_aspace_destroy(as);
	fake_clean("faux pmm propre");
}

int	main(void)
{
	h_begin("a03/ustack");
	h_run("ustack/pile-posee-d-un-bloc", pile_posee_d_un_bloc);
	h_run("ustack/table-pile-tenue", table_pile_tenue);
	h_run("ustack/retrait-dedie-bornes-exactes", retrait_dedie_bornes_exactes);
	h_run("ustack/echecs-sans-reste", echecs_sans_reste);
	return (h_end());
}
