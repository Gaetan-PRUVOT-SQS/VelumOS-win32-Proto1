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

static void	activate(t_shell *sh, int32_t idx)
{
	uint32_t	action;

	action = desk_icon((uint32_t)idx)->action;
	if (action == DA_RECYCLE)
		sh_desk_trash_open(sh);
	else
		sh_desk_action(sh, action);
}

static void	press(t_shell *sh, const t_wmmouse *m)
{
	t_click	now;
	int32_t	hit;

	sh_desk_sync(sh);
	hit = desk_pick(&sh->desk_state, sh->cells, sh->placed, m->x, m->y);
	select_cell(sh, hit);
	sh_desk_press(sh, hit, m);
	now = (t_click){hit, m->x, m->y, os_mono_ns()};
	if (dc_feed(&sh->last_click, &now) && hit >= 0)
		activate(sh, hit);
}

void	sh_desk_mouse(t_shell *sh, const t_wmmouse *m)
{
	if (m->type == INP_MOUSE_MOVE)
		sh_desk_move(sh, m);
	else if (m->type == INP_MOUSE_UP)
		sh_desk_release(sh, m);
	else if (m->type == INP_MOUSE_DOWN
		&& ((m->buttons >> WM_BTN_SHIFT) & WM_BTN_MASK) == BTN_LEFT)
		press(sh, m);
}

static void	delete_selected(t_shell *sh)
{
	int32_t	idx;

	idx = sh->selected;
	if (idx < 0)
		return ;
	if (desk_icon((uint32_t)idx)->action == DA_RECYCLE)
		sh_desk_trash_empty(sh);
	else if (desk_trash_put(&sh->desk_state, (uint32_t)idx) == 0)
	{
		sh->selected = DESK_NONE;
		sh_desk_paint(sh);
	}
}

void	sh_desk_key(t_shell *sh, uint32_t vk)
{
	int32_t	rows;
	int32_t	next;

	sh_desk_sync(sh);
	if (vk == VK_RETURN && sh->selected >= 0)
	{
		activate(sh, sh->selected);
		return ;
	}
	if (vk == VK_DELETE)
	{
		delete_selected(sh);
		return ;
	}
	rows = desk_rows(lay_rect(0, 0, (int32_t)sh->info.screen_w,
				(int32_t)sh->info.screen_h - sh->lm.taskbar_h));
	next = desk_move(sh->selected, sh->placed, rows, vk);
	if (next >= 0 && !desk_visible(&sh->desk_state, (uint32_t)next))
		return ;
	select_cell(sh, next);
}
