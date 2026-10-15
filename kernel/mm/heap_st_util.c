#include "heap_int.h"
#include "heap_st.h"

void	st_fill(void *p, size_t n, uint8_t seed)
{
	uint8_t	*b;
	size_t	i;

	b = p;
	i = 0;
	while (i < n)
	{
		b[i] = (uint8_t)(seed + i * 31 + (i >> 8));
		i++;
	}
}

int	st_verify(const void *p, size_t n, uint8_t seed)
{
	const uint8_t	*b;
	size_t			i;

	b = p;
	i = 0;
	while (i < n)
	{
		if (b[i] != (uint8_t)(seed + i * 31 + (i >> 8)))
			return (1);
		i++;
	}
	return (0);
}

uint64_t	st_rand(t_st_rng *r)
{
	uint64_t	x;

	x = r->state;
	x ^= x >> 12;
	x ^= x << 25;
	x ^= x >> 27;
	r->state = x;
	return (x * 0x2545f4914f6cdd1dull);
}
