#include "fake_alloc.h"

void	fa_fill(void *p, size_t n, uint8_t seed)
{
	uint8_t	*b;
	size_t	i;

	b = p;
	i = 0;
	while (i < n)
	{
		b[i] = (uint8_t)(seed + i * 7);
		i++;
	}
}

int	fa_intact(const void *p, size_t n, uint8_t seed)
{
	const uint8_t	*b;
	size_t			i;

	b = p;
	i = 0;
	while (i < n)
	{
		if (b[i] != (uint8_t)(seed + i * 7))
			return (0);
		i++;
	}
	return (1);
}

uint64_t	fa_rng(uint64_t *state)
{
	uint64_t	x;

	x = *state;
	x ^= x << 13;
	x ^= x >> 7;
	x ^= x << 17;
	*state = x;
	return (x);
}
