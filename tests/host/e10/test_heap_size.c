#include <stdint.h>
#include "../harness.h"
#include "heap_int.h"

#define GIO 0x40000000ull
#define MIO 0x100000ull

typedef struct s_case
{
	const char	*name;
	uint64_t	ram;
	uint32_t	shift;
	uint32_t	large;
	uint64_t	window_mio;
}	t_case;

static const t_case	g_cases[] = {
{"zero", 0, 26, 65536, 448},
{"256 Mio", 256 * MIO, 26, 65536, 448},
{"2 Gio moins un octet", 2 * GIO - 1, 26, 65536, 448},
{"2 Gio", 2 * GIO, 27, 131072, 896},
{"4 Gio", 4 * GIO, 27, 131072, 896},
{"8 Gio moins un octet", 8 * GIO - 1, 27, 131072, 896},
{"8 Gio", 8 * GIO, 28, 262144, 1792},
{"16 Gio", 16 * GIO, 28, 262144, 1792},
{"64 Gio", 64 * GIO, 28, 262144, 1792},
{"maximum", UINT64_MAX, 28, 262144, 1792},
};

static void	layout_par_palier(void)
{
	t_heap_layout	lay;
	size_t			i;

	i = 0;
	while (i < sizeof(g_cases) / sizeof(g_cases[0]))
	{
		lay.base = 0;
		heap_layout_for_ram(g_cases[i].ram, &lay);
		h_eq_u64(g_cases[i].name, lay.slab_shift, g_cases[i].shift);
		h_eq_u64(g_cases[i].name, lay.large_pages, g_cases[i].large);
		h_eq_u64(g_cases[i].name, heap_window_bytes(&lay) / MIO,
			g_cases[i].window_mio);
		i++;
	}
}

static void	layout_toujours_valide(void)
{
	t_heap_layout	lay;
	uint64_t		ram;

	ram = 1;
	while (ram)
	{
		heap_layout_for_ram(ram, &lay);
		h_true(lay.slab_shift >= HEAP_SHIFT_MIN
			&& lay.slab_shift <= HEAP_SHIFT_MAX, "décalage dans les bornes");
		h_true(lay.large_pages >= HEAP_LARGE_MIN
			&& lay.large_pages <= HEAP_LARGE_MAX, "grands blocs dans les bornes");
		ram <<= 1;
	}
}

static void	layout_monotone(void)
{
	t_heap_layout	prev;
	t_heap_layout	cur;
	uint64_t		ram;

	heap_layout_for_ram(0, &prev);
	ram = 1;
	while (ram)
	{
		heap_layout_for_ram(ram, &cur);
		h_true(cur.slab_shift >= prev.slab_shift
			&& cur.large_pages >= prev.large_pages, "jamais plus petit");
		prev = cur;
		ram <<= 1;
	}
}

static void	bitmaps_couvrent_le_maximum(void)
{
	h_eq_u64("dalles de 4 Kio", (1ull << HEAP_SHIFT_MAX) >> 12, HEAP_WORDS_S0 * 64);
	h_eq_u64("dalles de 8 Kio", (1ull << HEAP_SHIFT_MAX) >> 13, HEAP_WORDS_S1 * 64);
	h_eq_u64("dalles de 16 Kio", (1ull << HEAP_SHIFT_MAX) >> 14, HEAP_WORDS_S2 * 64);
	h_eq_u64("grands blocs", HEAP_LARGE_MAX, HEAP_WORDS_L * 64);
	h_eq_u64("total", HEAP_WORDS_ALL,
		HEAP_WORDS_S0 + HEAP_WORDS_S1 + HEAP_WORDS_S2 + HEAP_WORDS_L);
}

int	main(void)
{
	h_begin("e10/heap_size");
	h_run("palier par RAM (limites 2 Gio et 8 Gio)", layout_par_palier);
	h_run("disposition valide pour toute puissance de 2", layout_toujours_valide);
	h_run("disposition monotone", layout_monotone);
	h_run("bitmaps couvrent le maximum", bitmaps_couvrent_le_maximum);
	return (h_end());
}
