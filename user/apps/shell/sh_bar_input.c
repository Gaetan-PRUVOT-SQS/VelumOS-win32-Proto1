#include "velum/abi/abi_input.h"
#include "../common/layout.h"
#include "shell.h"

void	sh_bar_relayout(t_shell *sh)
{
	sh->shown = (uint32_t)tb_layout(sh->reg.strip, sh->tasks.count, sh->btn);
	sh->hot_task = -1;
	sh_bar_paint(sh);
}

void	sh_task_click(t_shell *sh, int32_t idx)
{
	const t_task	*t;

	if (idx < 0 || (uint32_t)idx >= sh->tasks.count)
		return ;
	t = &sh->tasks.list[idx];
	if (t->state != WSTATE_MIN && t->active)
		wmc_set_state_id(t->id, WSTATE_MIN);
	else
		wmc_activate(t->id);
}

static void	hover(t_shell *sh, int32_t x, int32_t y)
{
	int32_t	task;
	int		start;

	task = tb_hit(sh->btn, sh->shown, x, y);
	start = lay_contains(sh->reg.start, x, y);
	if (task == sh->hot_task && start == sh->hot_start)
		return ;
	sh->hot_task = task;
	sh->hot_start = start;
	sh_bar_paint(sh);
}

void	sh_bar_mouse(t_shell *sh, const t_wmmouse *m)
{
	if (m->type == INP_MOUSE_MOVE)
	{
		hover(sh, m->x, m->y);
		return ;
	}
	if (m->type != INP_MOUSE_DOWN
		|| ((m->buttons >> WM_BTN_SHIFT) & WM_BTN_MASK) != BTN_LEFT)
		return ;
	if (lay_contains(sh->reg.start, m->x, m->y))
		sh_menu_toggle(sh);
	else
		sh_task_click(sh, tb_hit(sh->btn, sh->shown, m->x, m->y));
}
