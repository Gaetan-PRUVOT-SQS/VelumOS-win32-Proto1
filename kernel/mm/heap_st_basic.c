#include "heap_int.h"
#include "heap_st.h"

static const uint32_t	g_sizes[] = {0, 1, 16, 17, 100, 1000, 2048, 2049, 4096,
	30000, 1000000};
static const uint32_t	g_aligns[] = {16, 32, 64, 128, 512, 4096};

int	st_sizes(void)
{
	uint32_t		i;
	void			*p;
	t_heap_stats	a;
	t_heap_stats	b;

	i = 0;
	while (i < sizeof(g_sizes) / sizeof(g_sizes[0]))
	{
		heap_get_stats(&a);
		p = kmalloc(g_sizes[i]);
		if (!p || ((uintptr_t)p & 15))
			return (1);
		st_fill(p, g_sizes[i], (uint8_t)i);
		heap_get_stats(&b);
		if (st_live(&b) != st_live(&a) + 1 || st_verify(p, g_sizes[i],
				(uint8_t)i))
			return (2);
		kfree(p);
		i++;
	}
	return (0);
}

int	st_aligned(void)
{
	uint32_t	i;
	void		*p;

	i = 0;
	while (i < sizeof(g_aligns) / sizeof(g_aligns[0]))
	{
		p = kmalloc_aligned(100, g_aligns[i]);
		if (!p || ((uintptr_t)p & (g_aligns[i] - 1)))
			return (1);
		st_fill(p, 100, (uint8_t)i);
		if (st_verify(p, 100, (uint8_t)i))
			return (2);
		kfree(p);
		i++;
	}
	if (kmalloc_aligned(8, 0) || kmalloc_aligned(8, 3)
		|| kmalloc_aligned(8, 8192))
		return (3);
	return (0);
}

int	st_tags(void)
{
	t_heap_stats	a;
	t_heap_stats	b;
	void			*p[3];

	heap_get_stats(&a);
	p[0] = kmalloc_tag(20, HEAP_PROC);
	p[1] = kmalloc_tag(500, HEAP_FS);
	p[2] = kmalloc_tag(70000, HEAP_IPC);
	if (!p[0] || !p[1] || !p[2])
		return (1);
	heap_get_stats(&b);
	if (b.allocs_live[HEAP_PROC] != a.allocs_live[HEAP_PROC] + 1
		|| b.allocs_live[HEAP_FS] != a.allocs_live[HEAP_FS] + 1
		|| b.allocs_live[HEAP_IPC] != a.allocs_live[HEAP_IPC] + 1
		|| b.bytes_live[HEAP_IPC] < a.bytes_live[HEAP_IPC] + 70000)
		return (2);
	kfree(p[0]);
	kfree(p[1]);
	kfree(p[2]);
	heap_get_stats(&b);
	return (b.bytes_live[HEAP_FS] != a.bytes_live[HEAP_FS]);
}
