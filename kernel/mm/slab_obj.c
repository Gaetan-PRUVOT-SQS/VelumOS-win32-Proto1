#include "heap_int.h"

void	*slab_obj_take(const t_class *c, t_slab *s)
{
	uint64_t	*bits;
	uint32_t	w;
	uint32_t	bit;

	bits = slab_bits(s);
	w = s->hint;
	while (bits[w] == ~0ull)
		w++;
	bit = (uint32_t)__builtin_ctzll(~bits[w]);
	__atomic_store_n(&bits[w], bits[w] | (1ull << bit), __ATOMIC_RELAXED);
	s->hint = (uint16_t)w;
	s->inuse++;
	return (slab_obj_addr(c, s, w * 64 + bit));
}

uint32_t	slab_idx(const t_slab *s, const void *p, const void *at)
{
	const t_class	*c;
	uint32_t		off;

	c = &g_heap.cls[s->cls];
	if ((uintptr_t)p < (uintptr_t)s + c->obj_off)
		heap_fault("pointeur dans l'en-tête de dalle", p, at);
	off = (uint32_t)((uintptr_t)p - (uintptr_t)s - c->obj_off);
	if (off % c->size)
		heap_fault("pointeur qui n'est pas un début d'objet", p, at);
	if (off / c->size >= s->nobj)
		heap_fault("pointeur hors des objets de la dalle", p, at);
	return (off / c->size);
}

int	slab_live(const t_slab *s, uint32_t idx)
{
	uint64_t	word;

	word = __atomic_load_n(&slab_bits(s)[idx >> 6], __ATOMIC_RELAXED);
	return ((int)((word >> (idx & 63)) & 1));
}

int	slab_put(t_slab *s, uint32_t idx)
{
	uint64_t	*bits;

	bits = slab_bits(s);
	if (!slab_live(s, idx))
		return (-1);
	__atomic_store_n(&bits[idx >> 6], bits[idx >> 6] & ~(1ull << (idx & 63)),
		__ATOMIC_RELAXED);
	if ((idx >> 6) < s->hint)
		s->hint = (uint16_t)(idx >> 6);
	s->inuse--;
	return (0);
}
