#include "../common/layout.h"
#include "shell.h"

static t_rect	screen_area(const t_shell *sh)
{
	return (lay_rect(0, 0, (int32_t)sh->info.screen_w,
			(int32_t)sh->info.screen_h - sh->lm.taskbar_h));
}

static int32_t	skipped(const t_deskstate *st)
{
	if (st->dragging)
		return (st->press);
	return (DESK_NONE);
}

static void	reset_slots(t_deskstate *st)
{
	uint32_t	trashed;
	uint32_t	gone;

	trashed = st->trashed;
	gone = st->gone;
	desk_state_init(st);
	st->trashed = trashed;
	st->gone = gone;
}

void	sh_desk_sync(t_shell *sh)
{
	t_rect	slots[DESK_SLOTS_MAX];
	int32_t	n;

	if (!sh->desk_state.ready)
		desk_state_init(&sh->desk_state);
	n = desk_slots(screen_area(sh), slots, DESK_SLOTS_MAX);
	if (desk_apply(&sh->desk_state, slots, n, sh->cells,
			skipped(&sh->desk_state)) == 0)
		return ;
	reset_slots(&sh->desk_state);
	desk_apply(&sh->desk_state, slots, n, sh->cells,
		skipped(&sh->desk_state));
}

void	sh_desk_press(t_shell *sh, int32_t hit, const t_wmmouse *m)
{
	t_deskstate	*st;

	st = &sh->desk_state;
	desk_press_clear(st);
	if (hit < 0 || hit >= DESK_ICONS)
		return ;
	st->press = hit;
	st->press_x = m->x;
	st->press_y = m->y;
	st->grab_dx = m->x - sh->cells[hit].x;
	st->grab_dy = m->y - sh->cells[hit].y;
}

static int	beyond_slop(const t_deskstate *st, const t_wmmouse *m)
{
	int64_t	dx;
	int64_t	dy;

	dx = (int64_t)m->x - st->press_x;
	dy = (int64_t)m->y - st->press_y;
	return (dx < -DESK_DRAG_SLOP || dx > DESK_DRAG_SLOP
		|| dy < -DESK_DRAG_SLOP || dy > DESK_DRAG_SLOP);
}

void	sh_desk_move(t_shell *sh, const t_wmmouse *m)
{
	t_deskstate	*st;
	t_rect		area;
	t_rect		*c;

	st = &sh->desk_state;
	if (st->press == DESK_NONE || (!st->dragging && !beyond_slop(st, m)))
		return ;
	st->dragging = 1;
	dc_reset(&sh->last_click);
	area = screen_area(sh);
	c = &sh->cells[st->press];
	c->x = lay_clamp(m->x - st->grab_dx, area.x, area.x + area.w - c->w);
	c->y = lay_clamp(m->y - st->grab_dy, area.y, area.y + area.h - c->h);
	sh_desk_paint(sh);
}

static int32_t	recycle_index(const t_shell *sh)
{
	int32_t	i;

	i = 0;
	while (i < DESK_ICONS)
	{
		if (desk_visible(&sh->desk_state, (uint32_t)i)
			&& desk_icon((uint32_t)i)->action == DA_RECYCLE)
			return (i);
		i++;
	}
	return (DESK_NONE);
}

static void	drop_on_grid(t_shell *sh, int32_t icon)
{
	t_rect	slots[DESK_SLOTS_MAX];
	t_rect	c;
	int32_t	n;

	n = desk_slots(screen_area(sh), slots, DESK_SLOTS_MAX);
	c = sh->cells[icon];
	desk_drop(&sh->desk_state, (uint32_t)icon,
		desk_slot_near(slots, n, c.x + c.w / 2, c.y + c.h / 2));
}

void	sh_desk_release(t_shell *sh, const t_wmmouse *m)
{
	t_deskstate	*st;
	int32_t		icon;
	int32_t		bin;
	int			was_dragging;

	st = &sh->desk_state;
	icon = st->press;
	was_dragging = st->dragging;
	if (icon == DESK_NONE || !was_dragging)
	{
		desk_press_clear(st);
		return ;
	}
	desk_press_clear(st);
	bin = recycle_index(sh);
	if (bin != DESK_NONE && bin != icon && lay_contains(sh->cells[bin], m->x,
			m->y) && desk_trash_put(st, (uint32_t)icon) == 0)
		sh->selected = DESK_NONE;
	else
		drop_on_grid(sh, icon);
	sh_desk_sync(sh);
	sh_desk_paint(sh);
}

void	sh_desk_trash_empty(t_shell *sh)
{
	if (desk_trash_empty(&sh->desk_state) == 0)
	{
		sh_note(sh, "Corbeille", "La corbeille est vide.");
		return ;
	}
	sh_note(sh, "Corbeille", "La corbeille a été vidée.");
}

void	sh_desk_trash_open(t_shell *sh)
{
	t_rect	slots[DESK_SLOTS_MAX];
	int32_t	n;

	if (desk_trash_count(&sh->desk_state) == 0)
	{
		sh_note(sh, "Corbeille", "La corbeille est vide.");
		return ;
	}
	n = desk_slots(screen_area(sh), slots, DESK_SLOTS_MAX);
	desk_trash_restore(&sh->desk_state, n);
	sh_desk_sync(sh);
	sh_desk_paint(sh);
	sh_note(sh, "Corbeille", "Les éléments ont été restaurés sur le bureau.");
}
