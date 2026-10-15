#include "ctl_int.h"

static void	tri_v(t_surface *s, t_point apex, int32_t dy, t_color col)
{
	int32_t	i;

	i = 0;
	while (i < CTL_ARROW)
	{
		gfx_hline(s, (t_point){apex.x - i, apex.y + i * dy}, 2 * i + 1, col);
		i++;
	}
}

static void	tri_h(t_surface *s, t_point apex, int32_t dx, t_color col)
{
	int32_t	i;

	i = 0;
	while (i < CTL_ARROW)
	{
		gfx_vline(s, (t_point){apex.x + i * dx, apex.y - i}, 2 * i + 1, col);
		i++;
	}
}

void	ctl_sb_arrow(t_surface *s, t_rect r, int dir, t_color col)
{
	t_point	mid;

	mid.x = r.x + r.w / 2;
	mid.y = r.y + r.h / 2;
	if (dir == 0)
		tri_v(s, (t_point){mid.x, mid.y - CTL_ARROW / 2}, 1, col);
	else if (dir == 1)
		tri_v(s, (t_point){mid.x, mid.y + CTL_ARROW / 2 - 1}, -1, col);
	else if (dir == 2)
		tri_h(s, (t_point){mid.x - CTL_ARROW / 2, mid.y}, 1, col);
	else
		tri_h(s, (t_point){mid.x + CTL_ARROW / 2 - 1, mid.y}, -1, col);
}
