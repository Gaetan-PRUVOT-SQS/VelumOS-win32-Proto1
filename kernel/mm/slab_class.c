#include "heap_int.h"

static const uint16_t	g_sizes[HEAP_CLASSES] = {16, 32, 48, 64, 96, 128, 192,
	256, 384, 512, 768, 1024, 1536, 2048};
static const char		*g_names[HEAP_CLASSES] = {"heap.16", "heap.32",
	"heap.48", "heap.64", "heap.96", "heap.128", "heap.192", "heap.256",
	"heap.384", "heap.512", "heap.768", "heap.1024", "heap.1536", "heap.2048"};

static uint32_t	slab_fit(uint32_t size, uint32_t bytes, uint32_t *off)
{
	uint32_t	n;
	uint32_t	words;

	n = bytes / size;
	while (n > 0)
	{
		words = (n + 63) / 64;
		*off = (HEAP_SLAB_HDR + words * 8 + 63) & ~63u;
		if (*off + n * size <= bytes)
			return (n);
		n--;
	}
	return (0);
}

static void	class_shape(t_class *c, uint32_t size)
{
	uint32_t	i;
	uint32_t	bytes;

	i = 0;
	while (i < HEAP_SLAB_ARENAS)
	{
		bytes = PAGE_SIZE << i;
		c->arena = i;
		c->nobj = slab_fit(size, bytes, &c->obj_off);
		if ((uint64_t)c->nobj * size * 100
			>= (uint64_t)bytes * HEAP_SLAB_MIN_PCT)
			return ;
		i++;
	}
}

void	slab_class_init(void)
{
	uint32_t	i;
	uint32_t	j;
	uint32_t	k;

	i = 0;
	while (i < HEAP_CLASSES)
	{
		g_heap.cls[i].size = g_sizes[i];
		class_shape(&g_heap.cls[i], g_sizes[i]);
		heap_lock_init(&g_heap.cls[i].lock, g_names[i]);
		i++;
	}
	j = 0;
	k = 0;
	while (j < HEAP_BY16)
	{
		while (g_sizes[k] < j * HEAP_ALIGN_MIN)
			k++;
		g_heap.by16[j] = k;
		j++;
	}
}

uint32_t	slab_class_find(size_t need, size_t align)
{
	uint32_t	i;

	if (need > HEAP_SMALL_MAX)
		return (HEAP_CLASSES);
	i = g_heap.by16[(need + 15) >> 4];
	while (align > HEAP_ALIGN_MIN && i < HEAP_CLASSES
		&& g_heap.cls[i].size % align)
		i++;
	return (i);
}
