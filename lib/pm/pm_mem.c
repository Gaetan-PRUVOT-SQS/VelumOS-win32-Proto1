#include <stdlib.h>
#include "pm_int.h"

void	*pm_alloc(size_t size)
{
	return (malloc(size));
}

void	pm_free(void *ptr)
{
	free(ptr);
}
