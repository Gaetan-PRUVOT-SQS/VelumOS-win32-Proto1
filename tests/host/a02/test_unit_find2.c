#include <string.h>
#include "a02_fake.h"

static uint64_t	g_map[4];

static void	setup(void)
{
	memset(g_map, 0xff, sizeof(g_map));
	g_pmm.bits = g_map;
	g_pmm.words = 4;
	pmm_bits_fill(100, 110, 0);
}

static void	find_degenerate(void)
{
	t_pmm_req	q;

	setup();
	q.lo = 50;
	q.hi = 50;
	q.count = 1;
	q.align = 1;
	q.start = 50;
	h_eq_u64("intervalle vide", pmm_find_run(&q), PMM_NONE);
	q.lo = 60;
	q.hi = 50;
	h_eq_u64("intervalle inverse", pmm_find_run(&q), PMM_NONE);
	q.lo = 100;
	q.hi = 105;
	q.start = 100;
	q.count = 6;
	h_eq_u64("plus long que l'intervalle", pmm_find_run(&q), PMM_NONE);
	q.count = 5;
	h_eq_u64("juste la longueur de l'intervalle", pmm_find_run(&q), 100);
	pmm_reset();
}

static void	find_tail_window(void)
{
	t_pmm_req	q;

	memset(g_map, 0xff, sizeof(g_map));
	g_pmm.bits = g_map;
	g_pmm.words = 4;
	pmm_bits_fill(249, 250, 0);
	q.lo = 0;
	q.hi = 256;
	q.count = 8;
	q.align = 1;
	q.start = 251;
	h_eq_u64("une frame libre en queue, 8 demandees", pmm_find_run(&q),
		PMM_NONE);
	q.count = 1;
	h_eq_u64("une frame suffit", pmm_find_run(&q), 249);
	pmm_reset();
}

int	main(void)
{
	h_begin("a02/unit_find2");
	h_run("recherche/limite : intervalles degeneres", find_degenerate);
	h_run("recherche/limite : aucune lecture au-dela de la borne",
		find_tail_window);
	return (h_end());
}
