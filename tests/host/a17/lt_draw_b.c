#include "lt_draw.h"

void	lt_d_button(t_surface *s, t_rect r)
{
	t_lunabtn	b;
	int			i;

	i = 0;
	while (i < 6)
	{
		b.r = r;
		b.state = (t_lunastate)i;
		b.is_default = (i == 4);
		b.focus = (i % 2 == 1);
		luna_button(s, &b);
		i++;
	}
}

void	lt_d_check(t_surface *s, t_rect r)
{
	int	i;

	i = 0;
	while (i < 6)
	{
		luna_checkbox(s, lp_pt(r.x, r.y), (t_lunastate)i, i % 2 == 0);
		luna_radio(s, lp_pt(r.x, r.y), (t_lunastate)i, i % 2 == 1);
		i++;
	}
}

void	lt_d_edit(t_surface *s, t_rect r)
{
	luna_edit_frame(s, r, true);
	luna_edit_frame(s, r, false);
}

void	lt_d_group(t_surface *s, t_rect r)
{
	luna_groupbox(s, r, 40);
	luna_groupbox(s, r, 0);
	luna_groupbox(s, r, -7);
	luna_groupbox(s, r, INT32_MAX);
}

void	lt_d_progress(t_surface *s, t_rect r)
{
	luna_progress(s, r, 0);
	luna_progress(s, r, 37);
	luna_progress(s, r, 100);
	luna_progress(s, r, 4000000000u);
}
