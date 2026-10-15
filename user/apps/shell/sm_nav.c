#include "startmenu.h"

int32_t	sm_hover(t_startmenu *m, int32_t idx)
{
	if (!m->open)
		return (m->hot);
	if (idx < 0 || !sm_visible((uint32_t)idx))
		idx = SM_NO_ITEM;
	m->hot = idx;
	if (idx != SM_NO_ITEM)
		m->focus = SM_NO_ITEM;
	return (m->hot);
}

static int32_t	scan_visible(int32_t i, int32_t step)
{
	int32_t	tries;

	tries = 0;
	while (tries < SM_ITEMS)
	{
		i = (i + step) % SM_ITEMS;
		if (sm_visible((uint32_t)i))
			return (i);
		tries++;
	}
	return (SM_NO_ITEM);
}

int32_t	sm_move(t_startmenu *m, int dir)
{
	int32_t	found;
	int32_t	from;
	int32_t	step;

	if (!m->open)
		return (m->focus);
	from = m->focus;
	step = 1;
	if (dir < 0)
	{
		step = SM_ITEMS - 1;
		if (from < 0)
			from = 0;
	}
	found = scan_visible(from, step);
	if (found >= 0)
	{
		m->focus = found;
		m->hot = SM_NO_ITEM;
	}
	return (m->focus);
}
