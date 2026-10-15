#include "luna_int.h"

void	lp_dotted(t_surface *s, t_rect r, t_color c)
{
	int32_t	i;

	if (s == NULL || r.w <= 0 || r.h <= 0)
		return ;
	i = 0;
	while (i < r.w)
	{
		if (i % 2 == 0)
		{
			gfx_put(s, lp_pt(r.x + i, r.y), c);
			gfx_put(s, lp_pt(r.x + i, r.y + r.h - 1), c);
		}
		i++;
	}
	i = 0;
	while (i < r.h)
	{
		if (i % 2 == 0)
		{
			gfx_put(s, lp_pt(r.x, r.y + i), c);
			gfx_put(s, lp_pt(r.x + r.w - 1, r.y + i), c);
		}
		i++;
	}
}

static t_rect	arrow_row(t_rect r, int32_t dir, int32_t i)
{
	int32_t	cx;
	int32_t	cy;
	int32_t	len;

	cx = r.x + r.w / 2;
	cy = r.y + r.h / 2;
	len = 1 + 2 * i;
	if (dir == 1)
		len = 7 - 2 * i;
	if (dir == 0 || dir == 1)
		return (lp_rect(cx - len / 2, cy - 2 + i, len, 1));
	len = 1 + 2 * i;
	if (dir == 3)
		len = 7 - 2 * i;
	return (lp_rect(cx - 2 + i, cy - len / 2, 1, len));
}

void	lp_arrow(t_surface *s, t_rect r, int32_t dir, t_color c)
{
	int32_t	i;

	if (s == NULL)
		return ;
	i = 0;
	while (i < 4)
	{
		lp_fill(s, arrow_row(r, dir, i), c);
		i++;
	}
}
