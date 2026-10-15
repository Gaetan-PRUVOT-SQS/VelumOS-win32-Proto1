#include <string.h>
#include "a02_fake.h"

static uint64_t			g_map[4];
static const uint64_t	g_from[] = {0, 0, 63, 64, 1, 0, 5, 127, 190, 255};
static const uint64_t	g_to[] = {1, 64, 65, 128, 63, 256, 5, 129, 256, 256};

static void	unit_fill_set(void)
{
	size_t	k;

	g_pmm.bits = g_map;
	g_pmm.words = 4;
	k = 0;
	while (k < sizeof(g_from) / sizeof(g_from[0]))
	{
		memset(g_map, 0, sizeof(g_map));
		pmm_bits_fill(g_from[k], g_to[k], 1);
		h_eq_u64("remplissage a 1 exact",
			fake_ref_diff(g_map, g_from[k], g_to[k], 1), 0);
		k++;
	}
	pmm_reset();
}

static void	unit_fill_clear(void)
{
	size_t	k;

	g_pmm.bits = g_map;
	g_pmm.words = 4;
	k = 0;
	while (k < sizeof(g_from) / sizeof(g_from[0]))
	{
		memset(g_map, 0xff, sizeof(g_map));
		pmm_bits_fill(g_from[k], g_to[k], 0);
		h_eq_u64("remplissage a 0 exact",
			fake_ref_diff(g_map, g_from[k], g_to[k], 0), 0);
		k++;
	}
	pmm_reset();
}

static void	unit_fill_accumulates(void)
{
	g_pmm.bits = g_map;
	g_pmm.words = 4;
	memset(g_map, 0, sizeof(g_map));
	pmm_bits_fill(10, 20, 1);
	pmm_bits_fill(60, 70, 1);
	pmm_bits_fill(15, 65, 0);
	h_eq_u64("deux ilots restent : 10-14", fake_ref_next(g_map, 0, 256, 1),
		10);
	h_eq_u64("fin du premier ilot", fake_ref_next(g_map, 10, 256, 0), 15);
	h_eq_u64("deuxieme ilot", fake_ref_next(g_map, 15, 256, 1), 65);
	pmm_reset();
}

int	main(void)
{
	h_begin("a02/unit_bits");
	h_run("bits/limite : remplissage a 1 aux frontieres de mot",
		unit_fill_set);
	h_run("bits/limite : remplissage a 0 aux frontieres de mot",
		unit_fill_clear);
	h_run("bits/etat : remplissages successifs", unit_fill_accumulates);
	return (h_end());
}
