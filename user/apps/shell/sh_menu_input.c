#include "velum/abi/abi_input.h"
#include "../common/layout.h"
#include "shell.h"

void	sh_menu_key(t_shell *sh, uint32_t vk)
{
	sh_menu_run(sh, sm_key(&sh->sm, vk));
}

static void	menu_hover(t_shell *sh, int32_t item)
{
	int32_t	hot;
	int32_t	focus;

	hot = sh->sm.hot;
	focus = sh->sm.focus;
	sm_hover(&sh->sm, item);
	if (sh->sm.hot != hot || sh->sm.focus != focus)
		sh_menu_paint(sh);
}

void	sh_menu_mouse(t_shell *sh, const t_wmmouse *m)
{
	int32_t	item;

	if (!sh->sm.open)
		return ;
	sh_menu_geo(sh);
	if (sh_launch_ui_mouse(sh, m))
		return ;
	item = luna_launcher_hit(&sh->sml, m->x, m->y);
	if (m->type == INP_MOUSE_MOVE)
	{
		menu_hover(sh, item);
		return ;
	}
	if (m->type != INP_MOUSE_DOWN
		|| ((m->buttons >> WM_BTN_SHIFT) & WM_BTN_MASK) != BTN_LEFT)
		return ;
	if (!lay_contains(sh->sml.panel, m->x, m->y))
		sh_menu_close(sh);
	else
		sh_menu_run(sh, sm_activate(&sh->sm, item));
}
