#include <stdlib.h>
#include <string.h>
#include "velum/err.h"
#include "a02_fake.h"

#define CAP 2048

static void	one_each_owner(void)
{
	uint64_t	phys;
	int			o;

	h_eq_i64("boot", fake_simple(1, 8), 0);
	o = PMM_KERNEL;
	while (o < PMM_OWNERS)
	{
		phys = pmm_alloc((t_pmm_owner)o);
		h_true(phys >= MIB && is_aligned(phys, PAGE_SIZE), "alloc valide");
		h_eq_u64("compteur du proprietaire", fake_owned(o), 1);
		pmm_free(phys, (t_pmm_owner)o);
		h_eq_u64("compteur remis a zero", fake_owned(o), 0);
		o++;
	}
	h_eq_i64("invariants", pmm_check(), 0);
}

static void	one_zeroing(void)
{
	uint64_t	plain;
	uint64_t	zero;
	uint64_t	again;

	h_eq_i64("boot", fake_simple(1, 8), 0);
	plain = pmm_alloc(PMM_USER);
	h_true(pmm_st_frame_is(plain, 1, FAKE_POISON), "pmm_alloc ne touche pas");
	zero = pmm_alloc_zero(PMM_USER);
	h_true(zero != plain, "deux frames distinctes");
	h_true(pmm_st_frame_is(zero, 1, 0), "pmm_alloc_zero : 4096 octets nuls");
	pmm_free(zero, PMM_USER);
	g_pmm.hint = zero >> PAGE_SHIFT;
	again = pmm_alloc_zero(PMM_USER);
	h_eq_u64("frame reutilisee", again, zero);
	h_true(pmm_st_frame_is(again, 1, 0), "nulle malgre le poison 0xDD");
	pmm_free(again, PMM_USER);
	pmm_free(plain, PMM_USER);
}

static void	one_poison_on_free(void)
{
	uint64_t	phys;
	uint64_t	next;

	h_eq_i64("boot", fake_simple(1, 8), 0);
	phys = pmm_alloc(PMM_HEAP);
	memset(pmm_virt(phys), 0x11, PAGE_SIZE);
	h_true(pmm_st_frame_is(phys, 1, 0x11), "ecriture visible");
	pmm_free(phys, PMM_HEAP);
	h_true(pmm_st_frame_is(phys, 1, PMM_POISON), "0xDD apres liberation");
	next = pmm_alloc(PMM_HEAP);
	h_true(pmm_st_frame_is(next, 1, FAKE_POISON), "voisine intacte");
	pmm_free(next, PMM_HEAP);
}

static void	one_first_and_last(void)
{
	uint64_t	*list;
	uint64_t	n;

	h_eq_i64("boot", fake_simple(1, 5), 0);
	h_eq_u64("meta 1 page", g_pmm.meta_pages, 1);
	list = malloc(CAP * sizeof(uint64_t));
	n = fake_drain(PMM_STACK, list, CAP);
	h_eq_u64("1023 frames", n, 1023);
	h_eq_u64("premiere frame du pool", fake_list_min(list, n), MIB);
	h_eq_u64("derniere avant la meta", fake_list_max(list, n),
		5 * MIB - 2 * PAGE_SIZE);
	h_eq_u64("aucun doublon", fake_list_dups(list, n), 0);
	h_eq_u64("pool vide : echec", pmm_alloc(PMM_STACK), 0);
	fake_free_list(list, n, PMM_STACK);
	h_eq_u64("tout est revenu", fake_free_pages(), 1023);
	free(list);
}

int	main(void)
{
	h_begin("a02/alloc_one");
	h_run("alloc/partition : une frame par proprietaire", one_each_owner);
	h_run("alloc/partition : zero ou non", one_zeroing);
	h_run("free/etat : poison 0xDD en debug", one_poison_on_free);
	h_run("alloc/limite : premiere et derniere frame", one_first_and_last);
	return (h_end());
}
