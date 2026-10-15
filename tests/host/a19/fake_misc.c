#include "fake.h"

static uint32_t	g_seed = 1;

bool	fake_click(t_ctlroot *r, int x, int y)
{
	bool	a;
	bool	b;

	a = fake_down(r, x, y);
	b = fake_up(r, x, y);
	return (a || b);
}

bool	fake_rect_eq(t_rect a, t_rect b)
{
	return (a.x == b.x && a.y == b.y && a.w == b.w && a.h == b.h);
}

void	fake_seed(uint32_t seed)
{
	g_seed = seed;
	if (g_seed == 0)
		g_seed = 1;
}

uint32_t	fake_rand(void)
{
	g_seed ^= g_seed << 13;
	g_seed ^= g_seed >> 17;
	g_seed ^= g_seed << 5;
	return (g_seed);
}
