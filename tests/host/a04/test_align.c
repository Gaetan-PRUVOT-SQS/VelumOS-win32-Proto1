#include <stdint.h>
#include "a04_fake.h"

static const uint16_t	g_aligns[] = {1, 2, 4, 8, 16, 32, 64, 128, 256, 512,
	1024, 2048, 4096};
static const uint16_t	g_sizes[] = {1, 17, 40, 100, 1000, 2048, 3000, 9000};
static const uint32_t	g_bad[] = {0, 3, 24, 48, 6000, 8192, 65536, 1u << 20};

static void	align_valid_pairs(void)
{
	size_t	a;
	size_t	s;
	void	*p;

	a04_fresh();
	a = 0;
	while (a < sizeof(g_aligns) / sizeof(g_aligns[0]))
	{
		s = 0;
		while (s < sizeof(g_sizes) / sizeof(g_sizes[0]))
		{
			p = kmalloc_aligned(g_sizes[s], g_aligns[a]);
			h_true(p != NULL, "E2 allocation alignee");
			h_true(((uintptr_t)p & (g_aligns[a] - 1)) == 0, "E2 alignement");
			h_true(((uintptr_t)p & 15) == 0, "E2 alignement 16 conserve");
			a04_fill(p, g_sizes[s], (uint8_t)(a + s));
			h_eq_i64("E2 donnees", a04_verify(p, g_sizes[s], (uint8_t)(a + s)),
				0);
			kfree(p);
			s++;
		}
		a++;
	}
	a04_drain("E2 paires valides");
}

static void	align_invalid_values(void)
{
	size_t			i;
	t_heap_stats	st;
	uint64_t		fails;

	a04_fresh();
	heap_get_stats(&st);
	fails = st.fail_calls;
	i = 0;
	while (i < sizeof(g_bad) / sizeof(g_bad[0]))
	{
		h_true(kmalloc_aligned(64, g_bad[i]) == NULL, "E2 alignement refuse");
		i++;
	}
	heap_get_stats(&st);
	h_eq_u64("E2 echecs comptes", st.fail_calls - fails,
		sizeof(g_bad) / sizeof(g_bad[0]));
	a04_drain("E2 valeurs invalides");
}

static void	align_page_alignment_is_exact(void)
{
	void	*p;
	void	*q;

	a04_fresh();
	p = kmalloc_aligned(8, 4096);
	q = kmalloc_aligned(5000, 4096);
	h_true(((uintptr_t)p & 4095) == 0 && ((uintptr_t)q & 4095) == 0,
		"E2 page");
	h_true(p != q, "E2 deux blocs distincts");
	a04_fill(q, 5000, 9);
	h_eq_i64("E2 donnees du bloc de 5000", a04_verify(q, 5000, 9), 0);
	kfree(p);
	kfree(q);
	a04_drain("E2 page");
}

static void	align_small_alignments_stay_in_slabs(void)
{
	void	*p[4];

	a04_fresh();
	p[0] = kmalloc_aligned(20, 32);
	p[1] = kmalloc_aligned(50, 64);
	p[2] = kmalloc_aligned(100, 64);
	p[3] = kmalloc_aligned(100, 16);
	h_true(a04_is_slab(p[0]) && a04_is_slab(p[1]), "E2 32 et 64 en dalle");
	h_true(a04_is_slab(p[2]) && a04_is_slab(p[3]), "E2 100 octets en dalle");
	h_eq_u64("E2 octets", a04_bytes(), 32 + 64 + 128 + a04_slot(100));
	kfree(p[0]);
	kfree(p[1]);
	kfree(p[2]);
	kfree(p[3]);
	a04_drain("E2 dalles alignees");
}

int	main(void)
{
	h_begin("a04/alignement");
	h_run("E2 paires taille et alignement valides", align_valid_pairs);
	h_run("E2 alignements invalides", align_invalid_values);
	h_run("E2 alignement de page", align_page_alignment_is_exact);
	h_run("E2 petits alignements en dalle",
		align_small_alignments_stay_in_slabs);
	return (h_end());
}
