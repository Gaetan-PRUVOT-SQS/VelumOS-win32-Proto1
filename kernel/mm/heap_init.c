#include "heap_int.h"
#include "velum/err.h"
#include "velum/klog.h"
#include "velum/libk.h"

t_heap					g_heap;

static const uint16_t	g_word_off[HEAP_ARENAS] = {0, HEAP_WORDS_S0,
	HEAP_WORDS_S0 + HEAP_WORDS_S1,
	HEAP_WORDS_S0 + HEAP_WORDS_S1 + HEAP_WORDS_S2};
static const char		*g_arena_names[HEAP_ARENAS] = {"heap.arena.4k",
	"heap.arena.8k", "heap.arena.16k", "heap.arena.large"};

_Static_assert(sizeof(t_slab) == HEAP_SLAB_HDR, "en-tête de dalle");
_Static_assert(sizeof(t_large) <= HEAP_LARGE_HDR, "en-tête de bloc");

static int	heap_layout_ok(const t_heap_layout *lay)
{
	return (lay->slab_shift >= HEAP_SHIFT_MIN
		&& lay->slab_shift <= HEAP_SHIFT_MAX
		&& lay->large_pages >= HEAP_LARGE_MIN
		&& lay->large_pages <= HEAP_LARGE_MAX
		&& (lay->base & (PAGE_SIZE - 1)) == 0);
}

static void	heap_arenas_init(void)
{
	t_arena_cfg	cfg;
	uint32_t	i;

	i = 0;
	while (i < HEAP_ARENAS)
	{
		cfg.shift = PAGE_SHIFT + i;
		cfg.base = g_heap.lay.base + ((uintptr_t)i << g_heap.lay.slab_shift);
		cfg.slots = (1ull << g_heap.lay.slab_shift) >> cfg.shift;
		if (i == HEAP_LARGE_ARENA)
		{
			cfg.shift = PAGE_SHIFT;
			cfg.slots = g_heap.lay.large_pages;
		}
		cfg.bits = g_heap.bits + g_word_off[i];
		cfg.name = g_arena_names[i];
		arena_init(&g_heap.arena[i], &cfg);
		i++;
	}
}

int	heap_boot_init(void)
{
	t_heap_layout	lay;
	int				rc;

	if (g_heap.ready)
		return (0);
	rc = heap_pages_layout(&lay);
	if (rc < 0)
		return (rc);
	if (!heap_layout_ok(&lay))
		return (E_INVAL);
	memset(&g_heap, 0, sizeof(g_heap));
	g_heap.lay = lay;
	heap_arenas_init();
	slab_class_init();
	heap_lock_init(&g_heap.large.lock, "heap.large");
	g_heap.fail_after = -1;
	g_heap.ready = 1;
	klog_info("heap: %u classes, dalles 4/8/16 Kio, %u Kio de bitmaps",
		HEAP_CLASSES,
		(unsigned)((sizeof(g_heap.bits) + sizeof(g_heap.heads)) / 1024));
	return (0);
}
