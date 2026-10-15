#include "ctl_int.h"

void	*malloc(size_t size);
void	free(void *ptr);

void	*ctl_alloc(size_t size)
{
	void	*p;

	p = malloc(size);
	if (p)
		memset(p, 0, size);
	return (p);
}

void	ctl_free(void *ptr)
{
	free(ptr);
}
