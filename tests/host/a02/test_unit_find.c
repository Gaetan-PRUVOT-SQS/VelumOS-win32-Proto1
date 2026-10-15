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

static void	find_basic(void)
{
	t_pmm_req	q;

	setup();
	q.lo = 0;
	q.hi = 256;
	q.count = 4;
	q.align = 1;
	q.start = 0;
	h_eq_u64("4 libres : au debut de l'ilot", pmm_find_run(&q), 100);
	q.align = 8;
	h_eq_u64("alignee sur 8", pmm_find_run(&q), 104);
	q.align = 16;
	h_eq_u64("alignee sur 16 : rien", pmm_find_run(&q), PMM_NONE);
	q.align = 1;
	q.count = 10;
	h_eq_u64("10 libres : exactement l'ilot", pmm_find_run(&q), 100);
	q.count = 11;
	h_eq_u64("11 libres : trop long", pmm_find_run(&q), PMM_NONE);
	pmm_reset();
}

static void	find_start_positions(void)
{
	t_pmm_req	q;

	setup();
	q.lo = 0;
	q.hi = 256;
	q.count = 4;
	q.align = 1;
	q.start = 105;
	h_eq_u64("depart dans l'ilot", pmm_find_run(&q), 105);
	q.start = 107;
	h_eq_u64("il reste moins de 4 apres : retour au debut", pmm_find_run(&q),
		100);
	q.start = 200;
	h_eq_u64("depart apres l'ilot", pmm_find_run(&q), 100);
	q.start = 255;
	q.count = 1;
	h_eq_u64("depart sur la derniere frame", pmm_find_run(&q), 100);
	pmm_reset();
}

int	main(void)
{
	h_begin("a02/unit_find");
	h_run("recherche/table : longueur et alignement", find_basic);
	h_run("recherche/table : position de depart", find_start_positions);
	return (h_end());
}
