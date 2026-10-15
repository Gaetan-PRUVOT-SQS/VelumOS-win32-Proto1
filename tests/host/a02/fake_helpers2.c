#include <stdlib.h>
#include <string.h>
#include "a02_fake.h"

static int	cmp_u64(const void *a, const void *b)
{
	uint64_t	x;
	uint64_t	y;

	x = *(const uint64_t *)a;
	y = *(const uint64_t *)b;
	return ((x > y) - (x < y));
}

uint64_t	fake_list_max(const uint64_t *list, uint64_t n)
{
	uint64_t	i;
	uint64_t	top;

	i = 0;
	top = 0;
	while (i < n)
	{
		top = max_u64(top, list[i]);
		i++;
	}
	return (top);
}

uint64_t	fake_list_min(const uint64_t *list, uint64_t n)
{
	uint64_t	i;
	uint64_t	low;

	i = 0;
	low = UINT64_MAX;
	while (i < n)
	{
		low = min_u64(low, list[i]);
		i++;
	}
	return (low);
}

uint64_t	fake_list_dups(const uint64_t *list, uint64_t n)
{
	uint64_t	*copy;
	uint64_t	i;
	uint64_t	dups;

	copy = malloc((n + 1) * sizeof(uint64_t));
	memcpy(copy, list, n * sizeof(uint64_t));
	qsort(copy, n, sizeof(uint64_t), cmp_u64);
	dups = 0;
	i = 1;
	while (i < n)
	{
		dups += (copy[i] == copy[i - 1]);
		i++;
	}
	free(copy);
	return (dups);
}

uint64_t	fake_inside(const uint64_t *l, uint64_t n, uint64_t a, uint64_t b)
{
	uint64_t	i;
	uint64_t	count;

	i = 0;
	count = 0;
	while (i < n)
	{
		count += (l[i] >= a && l[i] < b);
		i++;
	}
	return (count);
}
