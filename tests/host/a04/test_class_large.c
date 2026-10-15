#include <stdint.h>
#include "a04_fake.h"
#include "velum/libk.h"

static const uint32_t	g_sizes[] = {2048, 2049, 4000, 4032, 4033, 4095, 4096,
	4097, 8192, 65536, 1048576};

static void	large_partitions(void)
{
	void		*p;
	size_t		i;
	uint64_t	before;

	a04_fresh();
	i = 0;
	while (i < sizeof(g_sizes) / sizeof(g_sizes[0]))
	{
		before = a04_bytes();
		p = kmalloc(g_sizes[i]);
		h_true(p != NULL, "E1 gros : allocation");
		h_true(((uintptr_t)p & 15) == 0, "E1 gros : alignement 16");
		h_eq_u64("E1 gros : octets comptes", a04_bytes() - before,
			a04_slot(g_sizes[i]));
		a04_fill(p, g_sizes[i], (uint8_t)(i + 1));
		h_eq_i64("E1 gros : donnees intactes", a04_verify(p, g_sizes[i],
				(uint8_t)(i + 1)), 0);
		kfree(p);
		h_eq_u64("E1 gros : compteur de retour", a04_bytes(), before);
		i++;
	}
	a04_drain("E1 gros");
}

static void	large_guard_page_follows_block(void)
{
	char		*p;
	uintptr_t	end;

	a04_fresh();
	p = kmalloc(8000);
	end = ((uintptr_t)p - 64 + 2 * 4096);
	h_true(fake_pages_mapped_at((uintptr_t)p), "E1 garde : bloc mappe");
	h_true(fake_pages_mapped_at(end - 1), "E1 garde : derniere page mappee");
	h_true(!fake_pages_mapped_at(end), "E1 garde : page suivante non mappee");
	kfree(p);
	h_true(!fake_pages_mapped_at((uintptr_t)p), "E1 garde : rendu au vmm");
	a04_drain("E1 garde");
}

static void	large_too_big_is_refused(void)
{
	t_heap_stats	st;
	uint64_t		fails;

	a04_fresh();
	heap_get_stats(&st);
	fails = st.fail_calls;
	h_true(kmalloc((size_t)A04_LARGE * 4096) == NULL, "E1 plus que l'arene");
	h_true(kmalloc(SIZE_MAX) == NULL, "E1 SIZE_MAX");
	h_true(kmalloc(SIZE_MAX - 8) == NULL, "E1 SIZE_MAX - 8");
	h_true(kmalloc(((size_t)A04_LARGE - 2) * 4096) == NULL, "E1 limite haute");
	heap_get_stats(&st);
	h_eq_u64("E1 echecs comptes", st.fail_calls - fails, 4);
	a04_drain("E1 trop gros");
}

static void	large_largest_block_fits(void)
{
	void	*p;

	a04_fresh();
	p = kmalloc(((size_t)A04_LARGE - 3) * 4096);
	h_true(p != NULL, "E1 plus grand bloc possible");
	if (p)
		kfree(p);
	a04_drain("E1 plus grand bloc");
}

int	main(void)
{
	h_begin("a04/classes-grosses");
	h_run("E1 partitions des gros blocs", large_partitions);
	h_run("E1 page de garde apres le bloc", large_guard_page_follows_block);
	h_run("E1 demande trop grande", large_too_big_is_refused);
	h_run("E1 plus grand bloc", large_largest_block_fits);
	return (h_end());
}
