#include "heap_int.h"
#include "velum/libk.h"

static void	stats_class(t_heap_stats *out, t_class *c)
{
	uint64_t	flags;
	uint32_t	t;

	flags = heap_lock(&c->lock);
	t = 0;
	while (t < HEAP_TAGS)
	{
		out->bytes_live[t] += c->cache[t].bytes;
		out->allocs_live[t] += c->cache[t].allocs;
		t++;
	}
	out->alloc_calls += c->calls_alloc;
	out->free_calls += c->calls_free;
	heap_unlock(&c->lock, flags);
}

static void	stats_large(t_heap_stats *out)
{
	t_large_state	*l;
	uint64_t		flags;
	uint32_t		t;

	l = &g_heap.large;
	flags = heap_lock(&l->lock);
	t = 0;
	while (t < HEAP_TAGS)
	{
		out->bytes_live[t] += l->bytes[t];
		out->allocs_live[t] += l->allocs[t];
		t++;
	}
	out->alloc_calls += l->calls_alloc;
	out->free_calls += l->calls_free;
	heap_unlock(&l->lock, flags);
}

void	heap_get_stats(t_heap_stats *out)
{
	uint32_t	i;

	if (!out)
		return ;
	memset(out, 0, sizeof(*out));
	if (!g_heap.ready)
		return ;
	i = 0;
	while (i < HEAP_CLASSES)
	{
		stats_class(out, &g_heap.cls[i]);
		i++;
	}
	stats_large(out);
	out->fail_calls = __atomic_load_n(&g_heap.fail_calls, __ATOMIC_RELAXED);
	out->pages_mapped = __atomic_load_n(&g_heap.pages_mapped,
			__ATOMIC_RELAXED);
}
