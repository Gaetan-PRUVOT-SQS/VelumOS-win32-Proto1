#include "velum/abi/abi_input.h"
#include "shell.h"

void	sh_on_key(t_shell *sh, const t_wmkey *k)
{
	if (k->ev.type == INP_CHAR && !(k->ev.mods & (INPM_CTRL | INPM_ALT)))
		sh_launch_ui_char(sh, k->ev.code);
	if (k->ev.type != INP_KEY_DOWN)
		return ;
	if (k->ev.code == VK_LWIN || k->ev.code == VK_RWIN)
	{
		if (!(k->ev.mods & INPM_REPEAT))
			sh_menu_toggle(sh);
		return ;
	}
	if (sh->sm.open && !sh_launch_ui_key(sh, k->ev.code))
		sh_menu_key(sh, k->ev.code);
	else if (k->h.window == sh->desk.id)
		sh_desk_key(sh, k->ev.code);
}

void	sh_on_mouse(t_shell *sh, const t_wmmouse *m)
{
	if (m->h.window == sh->menu.id)
		sh_menu_mouse(sh, m);
	else if (m->h.window == sh->bar.id)
		sh_bar_mouse(sh, m);
	else if (m->h.window == sh->desk.id)
		sh_desk_mouse(sh, m);
}

int	sh_theme_sync(t_shell *sh)
{
	if (!sh_wall_sync(sh))
		return (0);
	sh_desk_paint(sh);
	sh_bar_paint(sh);
	if (sh->sm.open)
		sh_menu_paint(sh);
	return (1);
}
