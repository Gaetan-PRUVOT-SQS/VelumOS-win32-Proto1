#include "alloc_int.h"
#include "velum/err.h"
#include "velum/vmem.h"

int	alloc_chunk_new(void)
{
	int64_t	va;

	va = v_valloc(0, ALLOC_CHUNK, PROT_R | PROT_W);
	if (va < 0)
		return (0);
	alloc_recent_forget((uint64_t)va, (uint64_t)va + ALLOC_CHUNK);
	g_alloc.bump = (uint8_t *)(uintptr_t)va;
	g_alloc.bump_end = g_alloc.bump + ALLOC_CHUNK;
	g_alloc.stats.chunk_bytes += ALLOC_CHUNK;
	return (1);
}

void	*alloc_carve(uint32_t cls)
{
	size_t		stride;
	t_ablock	*b;

	stride = ALLOC_HDR + alloc_class_size(cls);
	if ((size_t)(g_alloc.bump_end - g_alloc.bump) < stride
		&& !alloc_chunk_new())
		return (NULL);
	b = (t_ablock *)g_alloc.bump;
	g_alloc.bump += stride;
	return (b);
}
