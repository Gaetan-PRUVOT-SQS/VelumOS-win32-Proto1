#include <string.h>
#include "a04_fake.h"
#include "velum/util.h"

uint32_t	a04_meta_size(t_rng *r)
{
	uint64_t	roll;

	roll = a04_rng_below(r, 100);
	if (roll < 60)
		return (1 + (uint32_t)a04_rng_below(r, 300));
	if (roll < 85)
		return (301 + (uint32_t)a04_rng_below(r, 1748));
	if (roll < 97)
		return (2049 + (uint32_t)a04_rng_below(r, 38000));
	return (40001 + (uint32_t)a04_rng_below(r, 260000));
}

static void	meta_take(t_meta *m, uint32_t j, void *p, uint64_t slot)
{
	m->p[j] = p;
	m->slot[j] = slot;
	m->expect += slot;
	m->live++;
	m->sum = m->sum * 31 + ((uintptr_t)p - g_heap.lay.base);
	a04_fill(p, m->n[j], (uint8_t)j);
}

void	a04_meta_alloc(t_meta *m, uint32_t j)
{
	size_t	n;
	size_t	al;
	void	*p;

	n = a04_meta_size(&m->rng);
	al = (size_t)16 << a04_rng_below(&m->rng, 9);
	m->n[j] = (uint32_t)n;
	if (a04_rng_below(&m->rng, 6) < 2)
	{
		p = kmalloc_aligned(n, al);
		h_true(p != NULL && ((uintptr_t)p & (al - 1)) == 0, "meta: aligne");
		meta_take(m, j, p, a04_slot_aligned(n, al));
		return ;
	}
	p = kmalloc_tag(n, (t_heap_tag)a04_rng_below(&m->rng, HEAP_TAGS));
	h_true(p != NULL, "meta: allocation");
	meta_take(m, j, p, a04_slot(n));
}
