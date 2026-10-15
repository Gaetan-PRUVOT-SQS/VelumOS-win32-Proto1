#include <stdint.h>
#include "alloc_int.h"
#include "fake_alloc.h"
#include "fake_sys.h"
#include "harness.h"
#include "stdlib.h"

#define FRAG_N 2000

static size_t	g_frag_sizes[FRAG_N];

static void	frag_alloc_all(void **p, uint64_t seed)
{
	int	i;

	i = 0;
	while (i < FRAG_N)
	{
		g_frag_sizes[i] = 1 + fa_rng(&seed) % 700;
		p[i] = malloc(g_frag_sizes[i]);
		i++;
	}
}

static void	frag_free_all(void **p)
{
	int	i;

	i = 0;
	while (i < FRAG_N)
	{
		free(p[i]);
		i++;
	}
}

static void	frag_reuse_without_growth(void)
{
	static void		*p[FRAG_N];
	t_vheapstats	st;

	fake_reset();
	fake_kernel_on();
	frag_alloc_all(p, 0x1234abcdull);
	v_heap_stats(&st);
	frag_free_all(p);
	frag_alloc_all(p, 0x1234abcdull);
	h_eq_u64("tranches inchangees", g_alloc.stats.chunk_bytes, st.chunk_bytes);
	frag_free_all(p);
	h_eq_u64("live_blocks", g_alloc.stats.live_blocks, 0);
	h_eq_u64("live_bytes", g_alloc.stats.live_bytes, 0);
}

static void	frag_alternate_free(void)
{
	static void		*p[FRAG_N];
	t_vheapstats	st;
	int				i;

	fake_reset();
	fake_kernel_on();
	frag_alloc_all(p, 0x77ull);
	v_heap_stats(&st);
	i = 0;
	while (i < FRAG_N)
	{
		free(p[i]);
		p[i] = malloc(g_frag_sizes[i]);
		i += 2;
	}
	h_eq_u64("tranches inchangees", g_alloc.stats.chunk_bytes, st.chunk_bytes);
	h_eq_u64("blocs vivants inchanges", g_alloc.stats.live_blocks,
		st.live_blocks);
	frag_free_all(p);
}

int	main(void)
{
	h_begin("a14/malloc_frag");
	h_run("malloc/flot de donnees : reutilisation", frag_reuse_without_growth);
	h_run("malloc/fragmentation : une sur deux", frag_alternate_free);
	return (h_end());
}
