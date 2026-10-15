#include "velum/libk.h"
#include "../common/utf8.h"
#include "tasklist.h"

static void	fill_task(t_task *t, const t_wminfo *info)
{
	t->id = info->h.window;
	t->pid = info->pid;
	t->style = info->style;
	t->state = info->state;
	t->icon = info->icon;
	t->active = (info->active != 0);
	utf8_clean_copy(t->title, TASK_TITLE_MAX, info->title,
		strnlen(info->title, WM_TITLE_MAX));
}

static void	keep_one_active(t_tasklist *l, int32_t keep)
{
	uint32_t	i;

	i = 0;
	while (i < l->count)
	{
		if ((int32_t)i != keep)
			l->list[i].active = 0;
		i++;
	}
}

static int	upsert(t_tasklist *l, const t_wminfo *info)
{
	int32_t	i;

	i = tl_find(l, info->h.window);
	if (!tl_shown(info->style, info->state))
	{
		if (i < 0)
			return (0);
		tl_remove(l, info->h.window);
		return (1);
	}
	if (i < 0 && l->count >= TASK_MAX)
		return (0);
	if (i < 0)
	{
		i = (int32_t)l->count;
		l->count++;
	}
	fill_task(&l->list[i], info);
	if (l->list[i].active)
		keep_one_active(l, i);
	return (1);
}

int	tl_apply(t_tasklist *l, uint32_t type, const t_wminfo *info)
{
	if (info->h.window == 0)
		return (0);
	if (type == WMS_WIN_DEL)
	{
		if (tl_find(l, info->h.window) < 0)
			return (0);
		tl_remove(l, info->h.window);
		return (1);
	}
	if (type != WMS_WIN_ADD && type != WMS_WIN_UPD)
		return (0);
	return (upsert(l, info));
}
