#include "a04_fake.h"

static const uint16_t	g_classes[HEAP_CLASSES] = {16, 32, 48, 64, 96, 128,
	192, 256, 384, 512, 768, 1024, 1536, 2048};

uint64_t	a04_bytes(void)
{
	t_heap_stats	st;
	uint64_t		sum;
	uint32_t		t;

	heap_get_stats(&st);
	sum = 0;
	t = 0;
	while (t < HEAP_TAGS)
		sum += st.bytes_live[t++];
	return (sum);
}

uint64_t	a04_objects(void)
{
	t_heap_stats	st;
	uint64_t		sum;
	uint32_t		t;

	heap_get_stats(&st);
	sum = 0;
	t = 0;
	while (t < HEAP_TAGS)
		sum += st.allocs_live[t++];
	return (sum);
}

uint64_t	a04_slot(size_t size)
{
	size_t		need;
	uint32_t	i;

	need = size;
	if (HEAP_DEBUG && size > 64)
		need = size + 16;
	i = 0;
	while (need <= 2048 && i < HEAP_CLASSES)
	{
		if (g_classes[i] >= need)
			return (g_classes[i]);
		i++;
	}
	return ((uint64_t)((size + 64 + 4095) / 4096) * 4096);
}

uint64_t	a04_slot_aligned(size_t size, size_t align)
{
	size_t		need;
	uint32_t	i;

	need = size;
	if (HEAP_DEBUG && size > 64)
		need = size + 16;
	i = 0;
	while (align <= 64 && need <= 2048 && i < HEAP_CLASSES)
	{
		if (g_classes[i] >= need && g_classes[i] % align == 0)
			return (g_classes[i]);
		i++;
	}
	if (align > 64)
		return ((uint64_t)((size + 4096 + 4095) / 4096) * 4096);
	return ((uint64_t)((size + 64 + 4095) / 4096) * 4096);
}

int	a04_is_slab(const void *p)
{
	uint32_t	arena;

	return (heap_locate((uintptr_t)p, &arena) == HEAP_KIND_SLAB);
}
