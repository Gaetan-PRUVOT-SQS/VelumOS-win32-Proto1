#include <stdlib.h>
#include "d01.h"

int	d01_same(const uint8_t *a, const uint8_t *b, size_t n)
{
	size_t	i;

	i = 0;
	while (i < n)
	{
		if (a[i] != b[i])
			return (0);
		i++;
	}
	return (1);
}

t_span	d01_span(const uint8_t *p, size_t n)
{
	t_span	s;

	s.p = p;
	s.len = n;
	return (s);
}

void	d01_mutate(uint8_t *p, size_t n, uint32_t *seed)
{
	uint32_t	k;

	k = 1 + d01_rand(seed) % 3;
	while (k-- > 0 && n > 0)
		p[d01_rand(seed) % n] ^= (uint8_t)(1 + d01_rand(seed) % 255);
}

int	d01_walk(const t_zip *z)
{
	t_zipent	e;
	uint8_t		*out;
	uint32_t	i;
	int			bad;

	bad = 0;
	i = 0;
	while (i < z->count)
	{
		if (zip_entry(z, i, &e) != 0)
			bad++;
		else if (e.usize <= (1u << 20))
		{
			out = malloc(e.usize);
			if (zip_extract(z, &e, out, e.usize) > (int64_t)e.usize)
				bad++;
			free(out);
		}
		i++;
	}
	return (bad);
}
