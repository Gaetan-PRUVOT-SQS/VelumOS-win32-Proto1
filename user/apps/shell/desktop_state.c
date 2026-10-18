#include "../common/layout.h"
#include "desktop.h"

void	desk_state_init(t_deskstate *st)
{
	int32_t	i;

	i = 0;
	while (i < DESK_ICONS)
	{
		st->slot[i] = i;
		i++;
	}
	st->trashed = 0;
	st->gone = 0;
	st->ready = 1;
	desk_press_clear(st);
}

void	desk_press_clear(t_deskstate *st)
{
	st->press = DESK_NONE;
	st->grab_dx = 0;
	st->grab_dy = 0;
	st->press_x = 0;
	st->press_y = 0;
	st->dragging = 0;
}

int	desk_visible(const t_deskstate *st, uint32_t icon)
{
	if (icon >= DESK_ICONS)
		return (0);
	return (((st->trashed | st->gone) & (1u << icon)) == 0);
}

int32_t	desk_owner(const t_deskstate *st, int32_t slot, int32_t except)
{
	int32_t	i;

	i = 0;
	while (i < DESK_ICONS)
	{
		if (i != except && desk_visible(st, (uint32_t)i) && st->slot[i] == slot)
			return (i);
		i++;
	}
	return (DESK_NONE);
}

int	desk_drop(t_deskstate *st, uint32_t icon, int32_t slot)
{
	int32_t	other;
	int32_t	from;

	if (icon >= DESK_ICONS || slot < 0 || !desk_visible(st, icon))
		return (0);
	from = st->slot[icon];
	if (from == slot)
		return (0);
	other = desk_owner(st, slot, (int32_t)icon);
	if (other != DESK_NONE)
		st->slot[other] = from;
	st->slot[icon] = slot;
	return (1);
}

int	desk_apply(const t_deskstate *st, const t_rect *slots, int32_t n,
	t_rect *cells, int32_t skip)
{
	int32_t	i;

	i = 0;
	while (i < DESK_ICONS)
	{
		if (st->slot[i] < 0 || st->slot[i] >= n)
			return (-1);
		i++;
	}
	i = 0;
	while (i < DESK_ICONS)
	{
		if (i != skip)
			cells[i] = slots[st->slot[i]];
		i++;
	}
	return (0);
}

int32_t	desk_pick(const t_deskstate *st, const t_rect *cells, uint32_t n,
	int32_t x, int32_t y)
{
	uint32_t	i;

	i = 0;
	while (i < n)
	{
		if (desk_visible(st, i) && lay_contains(cells[i], x, y))
			return ((int32_t)i);
		i++;
	}
	return (DESK_NONE);
}
