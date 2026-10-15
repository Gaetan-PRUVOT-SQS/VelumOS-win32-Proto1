#include "ref.h"

static t_rect	random_rect(int op)
{
	int32_t	x;
	int32_t	y;

	x = (int32_t)rng_range(0, RGN_G - 1);
	y = (int32_t)rng_range(0, RGN_G - 1);
	if (op == 0)
		return (rect_make(x, y, (int32_t)rng_range(0, RGN_G - x),
				(int32_t)rng_range(0, RGN_G - y)));
	return (rect_make(x - 4, y - 4, (int32_t)rng_range(0, RGN_G),
			(int32_t)rng_range(0, RGN_G)));
}

static int	random_op(void)
{
	int64_t	k;

	k = rng_range(0, 99);
	if (k < 62)
		return (0);
	if (k < 88)
		return (1);
	return (2);
}

void	rg_sequences(int count, int length, t_rgstat *st)
{
	t_region	rg;
	int			s;
	int			i;
	int			op;

	s = 0;
	while (s < count)
	{
		region_clear(&rg);
		i = 0;
		while (i < length)
		{
			op = random_op();
			rg_step(&rg, op, random_rect(op), st);
			i++;
		}
		s++;
	}
}
