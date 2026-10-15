#include "fake.h"

bool	fake_dirty_covers(const t_ctlroot *r, t_rect q)
{
	t_point		p;
	uint32_t	i;
	bool		hit;

	p.y = q.y;
	while (p.y < q.y + q.h)
	{
		p.x = q.x;
		while (p.x < q.x + q.w)
		{
			hit = false;
			i = 0;
			while (i < r->dirty.n)
			{
				hit = hit || rect_contains(r->dirty.r[i], p);
				i++;
			}
			if (!hit)
				return (false);
			p.x++;
		}
		p.y++;
	}
	return (true);
}

void	fake_px_fill(t_surface *s, uint32_t color)
{
	int	i;

	i = 0;
	while (i < s->w * s->h)
	{
		s->px[i] = color;
		i++;
	}
}
