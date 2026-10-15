#include "velum/abi/abi_input.h"
#include "../common/layout.h"
#include "../common/platform.h"
#include "shell.h"

static void	select_cell(t_shell *sh, int32_t idx)
{
	int32_t	old;

	if (idx == sh->selected)
		return ;
	old = sh->selected;
	sh->selected = idx;
	sh_desk_cell(sh, old);
	sh_desk_cell(sh, idx);
}

void	sh_desk_mouse(t_shell *sh, const t_wmmouse *m)
{
	t_click	now;
	int32_t	hit;

	if (m->type != INP_MOUSE_DOWN
		|| ((m->buttons >> WM_BTN_SHIFT) & WM_BTN_MASK) != BTN_LEFT)
		return ;
	hit = desk_hit(sh->cells, sh->placed, m->x, m->y);
	select_cell(sh, hit);
	now = (t_click){hit, m->x, m->y, os_mono_ns()};
	if (dc_feed(&sh->last_click, &now) && hit >= 0)
		sh_desk_action(sh, desk_icon((uint32_t)hit)->action);
}

void	sh_desk_key(t_shell *sh, uint32_t vk)
{
	int32_t	rows;

	if (vk == VK_RETURN && sh->selected >= 0)
	{
		sh_desk_action(sh, desk_icon((uint32_t)sh->selected)->action);
		return ;
	}
	rows = desk_rows(lay_rect(0, 0, (int32_t)sh->info.screen_w,
				(int32_t)sh->info.screen_h - sh->lm.taskbar_h));
	select_cell(sh, desk_move(sh->selected, sh->placed, rows, vk));
}
