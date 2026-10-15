#include <stdlib.h>
#include <string.h>
#include "fake.h"
#include "velum/heap.h"

static t_fakeheap	g_fh;

void	*kmalloc_tag(size_t size, t_heap_tag tag)
{
	uint64_t	*p;

	if (g_fh.armed && g_fh.left == 0)
		return (NULL);
	if (g_fh.armed)
		g_fh.left--;
	p = malloc(size + 16);
	if (p == NULL)
		return (NULL);
	p[0] = (uint64_t)tag;
	p[1] = size;
	memset(p + 2, 0xA5, size);
	g_fh.live[tag]++;
	return (p + 2);
}

void	kfree(void *ptr)
{
	uint64_t	*p;

	if (ptr == NULL)
		return ;
	p = (uint64_t *)ptr - 2;
	if (p[0] >= HEAP_TAGS || g_fh.live[p[0]] == 0)
		abort();
	g_fh.live[p[0]]--;
	free(p);
}

void	heap_fail_after(int64_t n)
{
	g_fh.armed = (n >= 0);
	g_fh.left = n;
}

void	heap_get_stats(t_heap_stats *out)
{
	uint32_t	i;

	memset(out, 0, sizeof(*out));
	i = 0;
	while (i < HEAP_TAGS)
	{
		out->allocs_live[i] = g_fh.live[i];
		i++;
	}
}

uint64_t	fake_heap_live(void)
{
	uint64_t	n;
	uint32_t	i;

	n = 0;
	i = 0;
	while (i < HEAP_TAGS)
		n += g_fh.live[i++];
	return (n);
}
