#include "alloc_int.h"

uint64_t	alloc_tag(const t_ablock *b, uint32_t state)
{
	uint64_t	t;

	t = g_alloc.seed ^ ALLOC_SEED_DEFAULT ^ (uint64_t)(uintptr_t)b;
	t ^= ((uint64_t)b->cls << 32) ^ b->pages ^ ((uint64_t)state << 16);
	t *= 0x9e3779b97f4a7c15ull;
	return (t ^ (t >> 29));
}

void	alloc_seal(t_ablock *b, uint32_t cls, uint32_t pages,
		uint32_t state)
{
	b->cls = cls;
	b->pages = pages;
	b->tag = alloc_tag(b, state);
}

uint32_t	alloc_state_of(const t_ablock *b)
{
	if (b->cls != ALLOC_CLS_LARGE && b->cls >= ALLOC_NCLASS)
		return (0);
	if (b->tag == alloc_tag(b, ALLOC_TAG_USED))
		return (ALLOC_TAG_USED);
	if (b->tag == alloc_tag(b, ALLOC_TAG_FREE))
		return (ALLOC_TAG_FREE);
	return (0);
}
