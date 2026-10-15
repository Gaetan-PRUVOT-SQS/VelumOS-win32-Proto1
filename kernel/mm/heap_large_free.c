#include "heap_int.h"

void	large_head_set(uint64_t page, int on)
{
	uint64_t	bit;

	bit = 1ull << (page & 63);
	if (on)
		__atomic_fetch_or(&g_heap.heads[page >> 6], bit, __ATOMIC_RELEASE);
	else
		__atomic_fetch_and(&g_heap.heads[page >> 6], ~bit, __ATOMIC_RELEASE);
}

int	large_head_test(uint64_t page)
{
	uint64_t	word;

	if (page >= g_heap.lay.large_pages)
		return (0);
	word = __atomic_load_n(&g_heap.heads[page >> 6], __ATOMIC_ACQUIRE);
	return ((int)((word >> (page & 63)) & 1));
}

t_large	*large_resolve(const void *p, const void *at)
{
	const t_arena	*a;
	uintptr_t		base;
	t_large			*h;

	a = &g_heap.arena[HEAP_LARGE_ARENA];
	base = (uintptr_t)p - HEAP_LARGE_HDR;
	if (((uintptr_t)p & (PAGE_SIZE - 1)) == 0)
		base = (uintptr_t)p - PAGE_SIZE;
	else if (((uintptr_t)p & (PAGE_SIZE - 1)) != HEAP_LARGE_HDR)
		heap_fault("pointeur qui n'est pas un début de bloc", p, at);
	if (base < a->base || !large_head_test(arena_index(a, base)))
		heap_fault("double libération ou pointeur étranger", p, at);
	h = (t_large *)base;
	if (h->cookie != (HEAP_LARGE_MAGIC ^ base) || h->data != (uintptr_t)p)
		heap_fault("pointeur qui n'est pas un début de bloc", p, at);
	return (h);
}

void	large_info(const void *p, const void *at, t_block *out)
{
	const t_large	*h;

	h = large_resolve(p, at);
	out->cap = ((size_t)h->npages << PAGE_SHIFT) - (h->data - (uintptr_t)h);
	out->used = h->size;
	out->tag = h->tag;
	out->kind = HEAP_KIND_LARGE;
	out->unit = h->npages;
}
