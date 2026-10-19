#include "desktop.h"

int	desk_deletable(uint32_t icon)
{
	const t_deskicon	*ic;

	ic = desk_icon(icon);
	return (ic != NULL && ic->action == DA_HELLO);
}

int	desk_trash_put(t_deskstate *st, uint32_t icon)
{
	if (!desk_deletable(icon) || !desk_visible(st, icon))
		return (-1);
	st->trashed |= 1u << icon;
	return (0);
}

uint32_t	desk_trash_count(const t_deskstate *st)
{
	uint32_t	i;
	uint32_t	n;

	n = 0;
	i = 0;
	while (i < DESK_ICONS)
	{
		if ((st->trashed & ~st->gone & (1u << i)) != 0)
			n++;
		i++;
	}
	return (n);
}

uint32_t	desk_trash_empty(t_deskstate *st)
{
	uint32_t	n;

	n = desk_trash_count(st);
	st->gone |= st->trashed;
	st->trashed = 0;
	return (n);
}

static int32_t	free_slot(const t_deskstate *st, int32_t nslots)
{
	int32_t	s;

	s = 0;
	while (s < nslots)
	{
		if (desk_owner(st, s, DESK_NONE) == DESK_NONE)
			return (s);
		s++;
	}
	return (DESK_NONE);
}

static int	restore_one(t_deskstate *st, uint32_t icon, int32_t nslots)
{
	int32_t	s;

	st->trashed &= ~(1u << icon);
	if (desk_owner(st, st->slot[icon], (int32_t)icon) == DESK_NONE)
		return (1);
	s = free_slot(st, nslots);
	if (s == DESK_NONE)
	{
		st->trashed |= 1u << icon;
		return (0);
	}
	st->slot[icon] = s;
	return (1);
}

uint32_t	desk_trash_restore(t_deskstate *st, int32_t nslots)
{
	uint32_t	i;
	uint32_t	n;

	n = 0;
	i = 0;
	while (i < DESK_ICONS)
	{
		if ((st->trashed & (1u << i)) != 0)
			n += (uint32_t)restore_one(st, i, nslots);
		i++;
	}
	return (n);
}
