#include "velum/err.h"
#include "ws_core.h"

bool	wf_visible(const t_wwin *w)
{
	return (w->id != 0 && w->state != WSTATE_MIN
		&& w->state != WSTATE_HIDDEN);
}

bool	wf_activatable(const t_wwin *w)
{
	return (wf_visible(w) && !(w->style & (WS_NOACTIVATE | WS_APPBAR)));
}

int	wf_next(const t_wtable *t, int exclude)
{
	uint32_t	i;
	int			slot;

	i = t->nz;
	while (i > 0)
	{
		i--;
		slot = t->z[i];
		if (slot != exclude && wf_activatable(&t->w[slot]))
			return (slot);
	}
	return (-1);
}

static int	check_order(const t_wtable *t)
{
	uint32_t	i;
	uint32_t	seen;

	i = 0;
	seen = 0;
	while (i < t->nz)
	{
		if (t->z[i] >= WS_WIN_MAX || t->w[t->z[i]].id == 0)
			return (E_INVAL);
		if (i > 0 && wz_band(t->w[t->z[i - 1]].style)
			> wz_band(t->w[t->z[i]].style))
			return (E_INVAL);
		if (wz_index(t, t->z[i]) != (int)i)
			return (E_INVAL);
		seen++;
		i++;
	}
	if (seen != t->nused)
		return (E_INVAL);
	return (0);
}

int	wz_check(const t_wtable *t)
{
	int	i;
	int	used;

	used = 0;
	i = 0;
	while (i < WS_WIN_MAX)
	{
		if (t->w[i].id != 0)
			used++;
		if (t->w[i].id != 0 && (int)(t->w[i].id & 0xff) != i + 1)
			return (E_INVAL);
		i++;
	}
	if ((uint32_t)used != t->nused || t->nz != t->nused)
		return (E_INVAL);
	if (t->active >= WS_WIN_MAX || (t->active >= 0
			&& !wf_activatable(&t->w[t->active])))
		return (E_INVAL);
	return (check_order(t));
}
