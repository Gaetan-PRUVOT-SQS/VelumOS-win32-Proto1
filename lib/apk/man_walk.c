#include "apk_int.h"

static int	on_activity(t_manwalk *w)
{
	int	r;

	w->in_act = 1;
	w->cand[0] = '\0';
	r = man_str(&w->x, AXML_ATTR_NAME, (t_text){w->cand, APK_CLASS_MAX});
	if (r == E_NOENT)
	{
		w->cand[0] = '\0';
		return (0);
	}
	return (r);
}

static void	on_filter_item(t_manwalk *w)
{
	char	v[APK_PERM_LEN];

	if (man_str(&w->x, AXML_ATTR_NAME, (t_text){v, sizeof(v)}) < 0)
		return ;
	if (strcmp(w->name, "action") == 0
		&& strcmp(v, "android.intent.action.MAIN") == 0)
		w->has_main = 1;
	if (strcmp(w->name, "category") == 0
		&& strcmp(v, "android.intent.category.LAUNCHER") == 0)
		w->has_launcher = 1;
}

int	man_on_child(t_manwalk *w)
{
	if (w->depth == 3 && w->in_app && strcmp(w->name, "activity") == 0)
		return (on_activity(w));
	if (w->depth == 4 && w->in_act && strcmp(w->name, "intent-filter") == 0)
		w->in_filter = 1;
	if (w->depth == 5 && w->in_filter)
		on_filter_item(w);
	return (0);
}

static int	on_end(t_manwalk *w)
{
	if (w->depth == 0)
		return (E_INVAL);
	if (w->depth == 4 && w->in_filter)
	{
		if (w->has_main && w->has_launcher && w->out->activity[0] == '\0')
			memcpy(w->out->activity, w->cand, APK_CLASS_MAX);
		w->in_filter = 0;
		w->has_main = 0;
		w->has_launcher = 0;
	}
	if (w->depth == 3)
		w->in_act = 0;
	if (w->depth == 2)
		w->in_app = 0;
	w->depth--;
	return (0);
}

int	man_walk(t_manwalk *w)
{
	int	ev;
	int	r;

	r = 0;
	ev = axml_next(&w->x);
	while (ev > 0 && r == 0)
	{
		if (ev == AXML_START)
		{
			if (axml_name(&w->x, (t_text){w->name, APK_ELEM_MAX}) < 0)
				w->name[0] = '\0';
			r = man_on_start(w);
		}
		else if (ev == AXML_END)
			r = on_end(w);
		if (r == 0)
			ev = axml_next(&w->x);
	}
	if (r < 0)
		return (r);
	return (ev);
}
