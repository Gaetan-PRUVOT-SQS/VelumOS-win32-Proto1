#include "velum/err.h"
#include "a02_fake.h"

static void	free_nominal(void)
{
	uint64_t	list[10];
	t_pmm_stats	s;
	uint64_t	n;

	h_eq_i64("boot", fake_simple(1, 8), 0);
	n = fake_drain(PMM_HEAP, list, 10);
	h_eq_u64("10 frames", n, 10);
	h_eq_u64("compteur HEAP", fake_owned(PMM_HEAP), 10);
	fake_free_list(list, n, PMM_HEAP);
	pmm_get_stats(&s);
	h_eq_u64("compteur HEAP revenu", s.owned[PMM_HEAP], 0);
	h_eq_u64("free_calls", s.free_calls, 10);
	h_eq_u64("alloc_calls", s.alloc_calls, 10);
	h_eq_i64("invariants", pmm_check(), 0);
}

static void	free_block_in_pieces(void)
{
	uint64_t	phys;

	h_eq_i64("boot", fake_simple(1, 8), 0);
	phys = pmm_alloc_pages(PMM_DMA, 8, 1, 0);
	pmm_free_pages(phys, 3, PMM_DMA);
	h_eq_u64("5 restent", fake_owned(PMM_DMA), 5);
	pmm_free(phys + 3 * PAGE_SIZE, PMM_DMA);
	pmm_free_pages(phys + 4 * PAGE_SIZE, 4, PMM_DMA);
	h_eq_u64("tout rendu", fake_owned(PMM_DMA), 0);
	h_eq_i64("invariants", pmm_check(), 0);
}

static void	free_zero_pages(void)
{
	t_pmm_stats	a;
	t_pmm_stats	b;

	h_eq_i64("boot", fake_simple(1, 8), 0);
	pmm_get_stats(&a);
	pmm_free_pages(0x123, 0, PMM_USER);
	pmm_free_pages(UINT64_MAX, 0, PMM_FREE);
	pmm_get_stats(&b);
	h_eq_i64("aucune assertion", fake_assert_count(), 0);
	h_eq_u64("free_calls inchange", b.free_calls, a.free_calls);
	h_eq_u64("libres inchanges", b.free_pages, a.free_pages);
}

int	main(void)
{
	h_begin("a02/free");
	h_run("free/partition : une frame a la fois", free_nominal);
	h_run("free/partition : bloc rendu en morceaux", free_block_in_pieces);
	h_run("free/limite : zero page", free_zero_pages);
	return (h_end());
}
