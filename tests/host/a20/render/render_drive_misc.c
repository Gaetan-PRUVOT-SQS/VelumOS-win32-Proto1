#include "render.h"

t_point	rect_center(t_rect r)
{
	return ((t_point){r.x + r.w / 2, r.y + r.h / 2});
}

void	drv_menu_item(t_shell *sh, int idx)
{
	drv_click(sh, sh->bar.id, rect_center(sh->reg.start));
	drv_click(sh, sh->menu.id, rect_center(sm_item_rect(&sh->sml, idx)));
}
