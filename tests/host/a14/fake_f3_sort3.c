#include "fake_alloc.h"
#include "fake_f3.h"

int	f3_is_sorted(const void *base, size_t n, size_t size,
		int (*cmp)(const void *, const void *))
{
	const uint8_t	*p;
	size_t			i;

	p = base;
	i = 1;
	while (i < n)
	{
		if (cmp(p + (i - 1) * size, p + i * size) > 0)
			return (0);
		i++;
	}
	return (1);
}

void	f3_hash(const void *base, size_t n, size_t size, uint64_t *out)
{
	const uint8_t	*p;
	uint64_t		h;
	size_t			i;
	size_t			j;

	p = base;
	out[0] = 0;
	out[1] = 0;
	i = 0;
	while (i < n)
	{
		h = 0xcbf29ce484222325ull;
		j = 0;
		while (j < size)
			h = (h ^ p[i * size + j++]) * 0x100000001b3ull;
		out[0] += h;
		out[1] ^= h;
		i++;
	}
}

void	f3_fill(void *base, size_t bytes, uint64_t *rng)
{
	uint8_t	*p;
	size_t	i;

	p = base;
	i = 0;
	while (i < bytes)
	{
		p[i] = (uint8_t)(fa_rng(rng) >> 24);
		i++;
	}
}
