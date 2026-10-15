#include "heap_int.h"
#include "velum/klog.h"
#include "velum/libk.h"

static int	check_cache(const t_class *c, uint32_t t)
{
	const t_cache	*k;
	t_walk			p;
	t_walk			f;
	int				bad;

	k = &c->cache[t];
	memset(&p, 0, sizeof(p));
	memset(&f, 0, sizeof(f));
	f.want_full = 1;
	slab_walk(c, &k->partial, t, &p);
	slab_walk(c, &k->full, t, &f);
	bad = p.bad + f.bad;
	bad += k->slabs != p.slabs + f.slabs;
	bad += k->empty != p.empty;
	bad += k->allocs != p.inuse + f.inuse;
	bad += k->bytes != k->allocs * c->size;
	return (bad);
}

static int	check_class(t_class *c)
{
	uint64_t	flags;
	uint32_t	t;
	int			bad;

	bad = 0;
	t = 0;
	flags = heap_lock(&c->lock);
	while (t < HEAP_TAGS)
	{
		bad += check_cache(c, t);
		t++;
	}
	heap_unlock(&c->lock, flags);
	if (bad)
		klog_err("heap: classe %u octets : %d anomalie(s)", c->size, bad);
	return (bad);
}

static int	check_arena(t_arena *a)
{
	uint64_t	flags;
	uint64_t	set;
	uint64_t	i;

	flags = heap_lock(&a->lock);
	set = 0;
	i = 0;
	while (i < (a->slots + 63) / 64)
	{
		set += (uint64_t)__builtin_popcountll(a->bits[i]);
		i++;
	}
	heap_unlock(&a->lock, flags);
	if (set != a->taken)
		klog_err("heap: arène %s : %llu bits pour %llu pris", a->lock.name,
			(unsigned long long)set, (unsigned long long)a->taken);
	return (set != a->taken);
}

int	heap_check(void)
{
	uint32_t	i;
	int			bad;

	if (!g_heap.ready)
		return (0);
	bad = 0;
	i = 0;
	while (i < HEAP_CLASSES)
	{
		bad += check_class(&g_heap.cls[i]);
		i++;
	}
	i = 0;
	while (i < HEAP_ARENAS)
	{
		bad += check_arena(&g_heap.arena[i]);
		i++;
	}
	return (bad + heap_check_large());
}
