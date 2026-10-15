#include "lt_draw.h"

void	lt_d_icons(t_surface *s, t_rect r)
{
	int	id;

	id = 0;
	while (id <= ICON_IDS)
	{
		luna_icon(s, lp_pt(r.x, r.y), (t_iconid)id, 16);
		luna_icon(s, lp_pt(r.x, r.y), (t_iconid)id, 32);
		luna_icon(s, lp_pt(r.x, r.y), (t_iconid)id, 48);
		luna_icon(s, lp_pt(r.x, r.y), (t_iconid)id, 3);
		luna_icon(s, lp_pt(r.x, r.y), (t_iconid)id, 513);
		luna_icon(s, lp_pt(r.x, r.y), (t_iconid)id, INT32_MAX);
		id++;
	}
	luna_icon(s, lp_pt(r.x, r.y), (t_iconid)-1, 16);
	luna_icon(s, lp_pt(r.x, r.y), (t_iconid)9999, 16);
}

void	lt_d_boot(t_surface *s, t_rect r)
{
	luna_boot_draw(s, (uint32_t)r.x, (uint32_t)r.y);
	luna_boot_draw(s, 4000000000u, 4000000000u);
}
