#include "velum/err.h"
#include "a02_fake.h"

static const uint64_t	g_base[] = {0x1000, 0x1001, 0x1800, 0, 0, 0x1000,
	0xfffffffffffff000ull, UINT64_MAX - 100};
static const uint64_t	g_len[] = {0x1000, 0x2000, 0x700, 0x5000, 0x5000, 0,
	0x1000, 50};
static const uint64_t	g_floor[] = {0x1000, 0x1000, 0x1000, 0x1000, 0x100000,
	0x1000, 0x1000, 0x1000};
static const uint64_t	g_lo[] = {0x1000, 0x2000, 0x2000, 0x1000, 0, 0x1000, 0,
	0};
static const uint64_t	g_hi[] = {0x2000, 0x3000, 0x2000, 0x5000, 0, 0x1000, 0,
	0};
static const int		g_ret[] = {1, 1, 0, 1, 0, 0, 0, 0};

static void	unit_piece_table(void)
{
	t_memrange	r;
	t_pmm_span	s;
	size_t		i;

	i = 0;
	while (i < sizeof(g_base) / sizeof(g_base[0]))
	{
		r.base = g_base[i];
		r.length = g_len[i];
		r.type = MEM_USABLE;
		s.lo = 0;
		s.hi = 0;
		h_eq_i64("pmm_piece : retour", pmm_piece(&r, g_floor[i], &s),
			g_ret[i]);
		if (g_ret[i])
		{
			h_eq_u64("pmm_piece : debut", s.lo, g_lo[i]);
			h_eq_u64("pmm_piece : fin", s.hi, g_hi[i]);
		}
		i++;
	}
}

static void	unit_range_end_and_types(void)
{
	t_memrange	r;
	uint32_t	t;

	r.base = 0x1000;
	r.length = 0x2000;
	h_eq_u64("fin normale", pmm_range_end(&r), 0x3000);
	r.base = UINT64_MAX - 5;
	r.length = 10;
	h_eq_u64("fin saturee", pmm_range_end(&r), UINT64_MAX);
	r.base = 0;
	r.length = UINT64_MAX;
	h_eq_u64("longueur maximale", pmm_range_end(&r), UINT64_MAX);
	t = MEM_USABLE;
	while (t <= MEM_FRAMEBUFFER + 1)
	{
		h_eq_i64("seuls USABLE et BOOT_RECLAIM sont du pool",
			pmm_is_pool_type(t), t == MEM_USABLE || t == MEM_BOOT_RECLAIM);
		t++;
	}
}

static void	unit_meta_pages_table(void)
{
	h_eq_u64("1 frame", pmm_meta_pages(1), 1);
	h_eq_u64("3640 frames : exactement une page", pmm_meta_pages(3640), 1);
	h_eq_u64("3641 frames : deux pages", pmm_meta_pages(3641), 2);
	h_eq_u64("65536 frames", pmm_meta_pages(65536), 18);
	h_eq_u64("2^20 frames", pmm_meta_pages(1ull << 20), 288);
	h_eq_u64("2^24 frames", pmm_meta_pages(1ull << 24), 4608);
	h_eq_u64("2^40 frames", pmm_meta_pages(1ull << 40), 301989888);
}

int	main(void)
{
	h_begin("a02/unit_range");
	h_run("plage/table : alignement vers l'interieur", unit_piece_table);
	h_run("plage/limite : fin saturee, types du pool",
		unit_range_end_and_types);
	h_run("plage/limite : taille des metadonnees", unit_meta_pages_table);
	return (h_end());
}
