#include <stdlib.h>
#include "fake.h"

void	*kmalloc_tag(size_t size, t_heap_tag tag)
{
	uint64_t	*p;

	if (g_fk.heap_fail > 0)
	{
		g_fk.heap_fail--;
		if (g_fk.heap_fail == 0)
			return (NULL);
	}
	p = malloc(size + 16);
	if (!p)
		return (NULL);
	p[0] = (uint64_t)tag;
	g_fk.heap_live[tag]++;
	return (p + 2);
}

void	kfree(void *ptr)
{
	uint64_t	*p;

	if (!ptr)
		return ;
	p = (uint64_t *)ptr - 2;
	g_fk.heap_live[p[0]]--;
	free(p);
}

int64_t	fk_heap_total(void)
{
	int64_t	total;
	int		i;

	total = 0;
	i = 0;
	while (i < HEAP_TAGS)
	{
		total += g_fk.heap_live[i];
		i++;
	}
	return (total);
}
