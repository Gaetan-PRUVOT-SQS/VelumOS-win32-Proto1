#include "heap_int.h"
#include "velum/klog.h"
#include "velum/libk.h"

int	large_check(const t_large *h)
{
	const t_arena	*a;
	uintptr_t		lead;
	int				bad;

	a = &g_heap.arena[HEAP_LARGE_ARENA];
	lead = h->data - (uintptr_t)h;
	bad = 0;
	bad += h->cookie != (HEAP_LARGE_MAGIC ^ (uintptr_t)h);
	bad += lead != HEAP_LARGE_HDR && lead != PAGE_SIZE;
	bad += h->tag >= HEAP_TAGS;
	bad += h->npages == 0 || h->size > ((uint64_t)h->npages << PAGE_SHIFT);
	bad += !large_head_test(arena_index(a, (uintptr_t)h));
	bad += !arena_held(a, arena_index(a, (uintptr_t)h), h->npages + 1ull);
	if (HEAP_DEBUG && bad == 0)
		bad += heap_dbg_large_bad(h);
	return (bad);
}

static int	large_plausible(const t_large *h)
{
	const t_arena	*a;
	uintptr_t		off;

	a = &g_heap.arena[HEAP_LARGE_ARENA];
	off = (uintptr_t)h - a->base;
	if ((uintptr_t)h < a->base || off >= (a->slots << a->shift)
		|| (off & (PAGE_SIZE - 1)) != 0)
		return (0);
	return (large_head_test(arena_index(a, (uintptr_t)h)));
}

static int	large_walk(const t_large_state *l, uint64_t *cnt, uint64_t *byt)
{
	const t_large	*h;
	uint64_t		n;
	int				bad;

	h = l->head;
	n = 0;
	bad = 0;
	while (h && n <= g_heap.lay.large_pages)
	{
		if (!large_plausible(h))
			return (bad + 1);
		bad += large_check(h) + (h->next && h->next->prev != h);
		if (h->tag < HEAP_TAGS)
		{
			cnt[h->tag]++;
			byt[h->tag] += (uint64_t)h->npages << PAGE_SHIFT;
		}
		n++;
		h = h->next;
	}
	return (bad + (h != NULL));
}

int	heap_check_large(void)
{
	uint64_t		cnt[HEAP_TAGS];
	uint64_t		byt[HEAP_TAGS];
	uint64_t		flags;
	uint32_t		t;
	int				bad;

	memset(cnt, 0, sizeof(cnt));
	memset(byt, 0, sizeof(byt));
	flags = heap_lock(&g_heap.large.lock);
	bad = large_walk(&g_heap.large, cnt, byt);
	t = 0;
	while (t < HEAP_TAGS)
	{
		bad += cnt[t] != g_heap.large.allocs[t];
		bad += byt[t] != g_heap.large.bytes[t];
		t++;
	}
	heap_unlock(&g_heap.large.lock, flags);
	if (bad)
		klog_err("heap: blocs larges : %d anomalie(s)", bad);
	return (bad);
}
