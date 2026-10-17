#include "velum/theme.h"
#include "../common/uitext.h"
#include "shell.h"

static t_lunastate	start_state(const t_shell *sh)
{
	if (sh->sm.open)
		return (LS_PRESSED);
	if (sh->hot_start)
		return (LS_HOT);
	return (LS_NORMAL);
}

static void	paint_back(t_shell *sh)
{
	t_surface	*s;
	t_blit		b;

	s = &sh->bar.surface;
	if (!sh->wall_px || sh->wall.h < s->h)
	{
		gfx_fill(s, rect_make(0, 0, s->w, s->h), SH_WALL_COLOR);
		return ;
	}
	b.dst = s;
	b.src = &sh->wall;
	b.dr = rect_make(0, 0, s->w, s->h);
	b.sr = rect_make(0, sh->wall.h - s->h, s->w, s->h);
	gfx_blit(&b);
}

static void	paint_task(t_shell *sh, uint32_t i)
{
	const t_task	*t;
	t_rect			b;
	t_uitext		st;
	t_lunastate		state;

	t = &sh->tasks.list[i];
	b = sh->btn[i];
	state = LS_NORMAL;
	if (t->active)
		state = LS_PRESSED;
	else if (sh->hot_task == (int32_t)i)
		state = LS_HOT;
	luna_task_button(&sh->bar.surface, b, state);
	luna_icon(&sh->bar.surface, (t_point){b.x + VT_TASK_PAD_L, b.y
		+ (b.h - sh->lm.icon_small) / 2}, (t_iconid)t->icon,
		sh->lm.icon_small);
	st.font = FONT_INS_SEMI_13;
	st.at.x = b.x + 2 * VT_TASK_PAD_L + sh->lm.icon_small;
	st.at.y = b.y + (b.h - ui_font_height(FONT_INS_SEMI_13)) / 2;
	st.color = sh_ink();
	st.max_w = b.x + b.w - VT_TASK_PAD_R - st.at.x;
	if (st.max_w > 8)
		ui_text(&sh->bar.surface, &st, t->title);
}

void	sh_bar_paint(t_shell *sh)
{
	t_surface	*s;
	uint32_t	i;

	s = &sh->bar.surface;
	paint_back(sh);
	luna_taskbar(s, rect_make(0, 0, s->w, s->h));
	luna_start_button(s, sh->reg.start, start_state(sh));
	i = 0;
	while (i < sh->shown)
	{
		paint_task(sh, i);
		i++;
	}
	sh_bar_tray(sh);
	wmc_present(&sh->bar, NULL, 0);
}
