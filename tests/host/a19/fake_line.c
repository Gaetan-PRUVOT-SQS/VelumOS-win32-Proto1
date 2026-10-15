#include "fake.h"

static int32_t	iabs(int32_t v)
{
	if (v < 0)
		return (-v);
	return (v);
}

void	gfx_line(t_surface *s, t_point a, t_point b, t_color c)
{
	int32_t	steps;
	int32_t	i;

	steps = iabs(b.x - a.x);
	if (iabs(b.y - a.y) > steps)
		steps = iabs(b.y - a.y);
	if (steps == 0)
	{
		gfx_put(s, a, c);
		return ;
	}
	i = 0;
	while (i <= steps)
	{
		gfx_put(s, (t_point){a.x + (b.x - a.x) * i / steps,
			a.y + (b.y - a.y) * i / steps}, c);
		i++;
	}
}

t_color	gfx_lerp(t_color a, t_color b, uint32_t t256)
{
	t_color		out;
	uint32_t	shift;
	uint32_t	ca;
	uint32_t	cb;

	out = 0;
	shift = 0;
	while (shift < 32)
	{
		ca = (a >> shift) & 0xff;
		cb = (b >> shift) & 0xff;
		out |= (((ca * (256 - t256) + cb * t256) >> 8) & 0xff) << shift;
		shift += 8;
	}
	return (out);
}
