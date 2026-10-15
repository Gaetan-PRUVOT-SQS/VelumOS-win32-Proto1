#include "a02_fake.h"

int	fake_ref_bit(const uint64_t *map, uint64_t i)
{
	return ((map[i / 64] >> (i % 64)) & 1);
}

static int	expected_bit(uint64_t i, const uint64_t *range, int inside)
{
	if (i >= range[0] && i < range[1])
		return (inside);
	return (!inside);
}

uint64_t	fake_ref_diff(const uint64_t *map, uint64_t from, uint64_t to,
				int inside)
{
	uint64_t	i;
	uint64_t	bad;
	uint64_t	range[2];

	range[0] = from;
	range[1] = to;
	i = 0;
	bad = 0;
	while (i < 256)
	{
		bad += (fake_ref_bit(map, i) != expected_bit(i, range, inside));
		i++;
	}
	return (bad);
}

uint64_t	fake_ref_next(const uint64_t *map, uint64_t from, uint64_t to,
				int one)
{
	while (from < to && fake_ref_bit(map, from) != one)
		from++;
	return (from);
}
