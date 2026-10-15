#include <stdlib.h>
#include "fake.h"
#include "velum/heap.h"

t_fkmem	g_fk;

void	*fk_track(void *p)
{
	int	i;

	if (!p)
		return (NULL);
	i = 0;
	while (i < FK_ALLOCS && g_fk.ptr[i])
		i++;
	if (i == FK_ALLOCS)
		abort();
	g_fk.ptr[i] = p;
	g_fk.live++;
	return (p);
}

void	fk_untrack(void *p)
{
	int	i;

	i = 0;
	while (i < FK_ALLOCS && g_fk.ptr[i] != p)
		i++;
	if (i == FK_ALLOCS)
		abort();
	g_fk.ptr[i] = NULL;
	g_fk.live--;
}

void	*kmalloc_tag(size_t size, t_heap_tag tag)
{
	(void)tag;
	if (fk_should_fail())
		return (NULL);
	return (fk_track(malloc(size + (size == 0))));
}

void	*kmalloc(size_t size)
{
	return (kmalloc_tag(size, HEAP_GENERIC));
}

void	kfree(void *ptr)
{
	if (!ptr)
		return ;
	fk_untrack(ptr);
	free(ptr);
}
