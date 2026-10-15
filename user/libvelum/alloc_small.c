#include "alloc_int.h"
#include "string.h"

static int	alloc_poison_ok(const t_ablock *b, uint32_t cls)
{
	const uint8_t	*p;
	size_t			n;

	p = (const uint8_t *)(b + 1);
	n = alloc_class_size(cls) - sizeof(uint64_t);
	if (n > ALLOC_POISON_CHECK)
		n = ALLOC_POISON_CHECK;
	p += sizeof(uint64_t);
	while (n--)
	{
		if (*p++ != ALLOC_POISON)
			return (0);
	}
	return (1);
}

static t_ablock	*alloc_pop(uint32_t cls)
{
	t_ablock	*b;
	uint64_t	link;

	b = g_alloc.free_head[cls];
	if (!b)
		return (NULL);
	if (alloc_state_of(b) != ALLOC_TAG_FREE || b->cls != cls
		|| !alloc_poison_ok(b, cls))
	{
		g_alloc.free_head[cls] = NULL;
		alloc_fault(ALLOC_FAULT_CORRUPT);
		return (NULL);
	}
	link = *(const uint64_t *)(b + 1) ^ g_alloc.seed ^ ALLOC_SEED_DEFAULT;
	g_alloc.free_head[cls] = (t_ablock *)(uintptr_t)link;
	if (link & (ALLOC_ALIGN - 1))
	{
		g_alloc.free_head[cls] = NULL;
		alloc_fault(ALLOC_FAULT_CORRUPT);
	}
	return (b);
}

void	*alloc_small_get(uint32_t cls)
{
	t_ablock	*b;

	b = alloc_pop(cls);
	if (!b)
		b = alloc_carve(cls);
	if (!b)
		return (NULL);
	alloc_seal(b, cls, 0, ALLOC_TAG_USED);
	g_alloc.stats.live_blocks++;
	g_alloc.stats.live_bytes += alloc_class_size(cls);
	return (b + 1);
}

void	alloc_small_put(t_ablock *b)
{
	uint32_t	cls;
	uint64_t	link;

	cls = b->cls;
	memset(b + 1, ALLOC_POISON, alloc_class_size(cls));
	alloc_seal(b, cls, 0, ALLOC_TAG_FREE);
	link = (uint64_t)(uintptr_t)g_alloc.free_head[cls];
	*(uint64_t *)(b + 1) = link ^ g_alloc.seed ^ ALLOC_SEED_DEFAULT;
	g_alloc.free_head[cls] = b;
	g_alloc.stats.live_blocks--;
	g_alloc.stats.live_bytes -= alloc_class_size(cls);
}
