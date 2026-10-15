#include "alloc_int.h"

static const uint16_t	g_alloc_sizes[ALLOC_NCLASS] = {16, 32, 48, 64, 96,
	128, 192, 256, 384, 512, 768, 1024, 1536, 2048};

int	alloc_class_of(size_t size)
{
	int	i;

	i = 0;
	while (i < ALLOC_NCLASS && size > g_alloc_sizes[i])
		i++;
	if (i == ALLOC_NCLASS)
		return (-1);
	return (i);
}

size_t	alloc_class_size(uint32_t cls)
{
	if (cls >= ALLOC_NCLASS)
		return (0);
	return (g_alloc_sizes[cls]);
}
