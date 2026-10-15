#include "velum/err.h"
#include "a02_fake.h"

static void	fault_reserved_frames(void)
{
	fake_reset();
	fake_range(MIB, 3 * MIB, MEM_USABLE);
	fake_range(4 * MIB, MIB, MEM_KERNEL);
	fake_range(5 * MIB, 3 * MIB, MEM_USABLE);
	h_eq_i64("boot", fake_boot(), 0);
	h_eq_i64("frame 0", fake_free_try(0, 1, PMM_KERNEL), 1);
	h_eq_str("message", fake_assert_msg(), "pmm: mauvais propriétaire");
	h_eq_i64("frame du noyau", fake_free_try(4 * MIB, 1, PMM_KERNEL), 1);
	h_eq_i64("frame sous 1 Mio", fake_free_try(0x5000, 1, PMM_KERNEL), 1);
	h_eq_i64("une libre jamais allouee", fake_free_try(MIB, 1, PMM_KERNEL),
		1);
	h_eq_str("message", fake_assert_msg(), "pmm: double libération");
	h_eq_i64("invariants", pmm_check(), 0);
}

static void	fault_metadata_frames(void)
{
	t_pmm_stats	a;
	t_pmm_stats	b;

	h_eq_i64("boot", fake_simple(1, 8), 0);
	pmm_get_stats(&a);
	h_eq_i64("liberer la meta", fake_free_try(g_pmm.meta_phys, 1,
			PMM_KERNEL), 1);
	h_eq_str("message", fake_assert_msg(), "pmm: mauvais propriétaire");
	h_eq_i64("liberer la meta, tous proprietaires",
		fake_free_try(g_pmm.meta_phys, 1, PMM_HEAP), 1);
	pmm_get_stats(&b);
	h_eq_u64("libres inchanges", b.free_pages, a.free_pages);
	h_eq_u64("meta toujours marquee", g_pmm.owner[g_pmm.meta_phys >> 12],
		PMM_MARK_META);
}

static void	fault_atomic_partial(void)
{
	uint64_t	block;
	uint64_t	a;
	uint64_t	b;

	h_eq_i64("boot", fake_simple(1, 8), 0);
	block = pmm_alloc_pages(PMM_USER, 4, 1, 0);
	h_eq_i64("5 frames dont une libre", fake_free_try(block, 5, PMM_USER), 1);
	h_eq_u64("les 4 restent allouees", fake_owned(PMM_USER), 4);
	a = pmm_alloc(PMM_KERNEL);
	b = pmm_alloc(PMM_HEAP);
	h_eq_u64("voisines", b, a + PAGE_SIZE);
	h_eq_i64("2 frames de proprietaires differents",
		fake_free_try(a, 2, PMM_KERNEL), 1);
	h_eq_u64("la premiere n'a pas ete liberee", fake_owned(PMM_KERNEL), 1);
	h_eq_i64("invariants", pmm_check(), 0);
}

int	main(void)
{
	h_begin("a02/free_faults2");
	h_run("free/table : frames reservees", fault_reserved_frames);
	h_run("free/table : frames de metadonnees", fault_metadata_frames);
	h_run("free/etat : echec atomique", fault_atomic_partial);
	return (h_end());
}
