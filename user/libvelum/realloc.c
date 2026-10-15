#include "alloc_int.h"
#include "stdlib.h"
#include "string.h"

void	*alloc_resize(void *p, size_t size)
{
	size_t	cap;
	void	*np;

	cap = alloc_probe(p);
	if (!cap)
		return (NULL);
	if (size <= cap && (size * 2 >= cap || cap <= ALLOC_ALIGN))
		return (p);
	np = malloc(size);
	if (!np)
		return (NULL);
	if (size < cap)
		cap = size;
	memcpy(np, p, cap);
	free(p);
	return (np);
}

void	*realloc(void *p, size_t size)
{
	if (!p)
		return (malloc(size));
	if (!size)
	{
		free(p);
		return (NULL);
	}
	return (alloc_resize(p, size));
}
