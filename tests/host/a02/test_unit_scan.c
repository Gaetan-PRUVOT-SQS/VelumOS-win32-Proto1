#include <string.h>
#include "a02_fake.h"

static uint64_t			g_map[4];
static const uint64_t	g_set[] = {0, 63, 64, 130, 255};
static const uint64_t	g_start[] = {0, 1, 63, 64, 65, 129, 130, 200, 255};
static const uint64_t	g_len[] = {0, 1, 2, 63, 64, 65, 128, 256};

static void	pattern(void)
{
	size_t	i;

	g_pmm.bits = g_map;
	g_pmm.words = 4;
	memset(g_map, 0, sizeof(g_map));
	i = 0;
	while (i < sizeof(g_set) / sizeof(g_set[0]))
	{
		pmm_bits_fill(g_set[i], g_set[i] + 1, 1);
		i++;
	}
}

static void	unit_next_all_pairs(void)
{
	size_t		i;
	size_t		j;
	uint64_t	to;
	int			one;

	pattern();
	i = 0;
	while (i < sizeof(g_start) / sizeof(g_start[0]))
	{
		j = 0;
		while (j < sizeof(g_len) / sizeof(g_len[0]))
		{
			to = min_u64(g_start[i] + g_len[j], 256);
			one = j & 1;
			h_eq_u64("pmm_bits_next = reference",
				pmm_bits_next(g_start[i], to, one),
				fake_ref_next(g_map, g_start[i], to, one));
			j++;
		}
		i++;
	}
	pmm_reset();
}

static void	unit_next_skips_whole_words(void)
{
	memset(g_map, 0xff, sizeof(g_map));
	g_pmm.bits = g_map;
	g_pmm.words = 4;
	g_pmm.scanned = 0;
	h_eq_u64("tout plein : rien de libre", pmm_bits_next(0, 256, 0), 256);
	h_eq_u64("un mot lu par mot de 64 bits", g_pmm.scanned, 4);
	g_pmm.scanned = 0;
	h_eq_u64("intervalle vide : aucune lecture", pmm_bits_next(70, 70, 0), 70);
	h_eq_u64("zero mot", g_pmm.scanned, 0);
	g_map[3] = ~(1ull << 40);
	h_eq_u64("le trou est trouve", pmm_bits_next(0, 256, 0), 3 * 64 + 40);
	pmm_reset();
}

static void	unit_count_free(void)
{
	memset(g_map, 0xff, sizeof(g_map));
	g_pmm.bits = g_map;
	g_pmm.words = 4;
	h_eq_u64("tout plein", pmm_bits_count_free(), 0);
	memset(g_map, 0, sizeof(g_map));
	h_eq_u64("tout libre", pmm_bits_count_free(), 256);
	memset(g_map, 0x55, sizeof(g_map));
	h_eq_u64("un sur deux", pmm_bits_count_free(), 128);
	g_map[0] = 1;
	g_map[1] = 0x8000000000000000ull;
	g_map[2] = 0xfffffffffffffffeull;
	g_map[3] = 0;
	h_eq_u64("mots particuliers", pmm_bits_count_free(), 63 + 63 + 1 + 64);
	pmm_reset();
}

int	main(void)
{
	h_begin("a02/unit_scan");
	h_run("scan/combinatoire : toutes les paires (depart, longueur)",
		unit_next_all_pairs);
	h_run("scan/limite : saut des mots pleins", unit_next_skips_whole_words);
	h_run("scan/limite : comptage des libres", unit_count_free);
	return (h_end());
}
