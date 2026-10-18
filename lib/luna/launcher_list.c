#include "launcher_int.h"

t_lmview	luna_lm_view(const t_lmodel *m)
{
	if (m == NULL)
		return (LUNA_LV_PINNED);
	if (m->qlen > 0)
		return (LUNA_LV_SEARCH);
	return ((t_lmview)m->tab);
}

void	luna_lm_set_tab(t_lmodel *m, int32_t tab)
{
	if (m == NULL)
		return ;
	m->tab = lp_clamp(tab, LUNA_LV_PINNED, LUNA_LV_ALL);
	lp_lm_refresh(m);
}

static void	list_search(t_lmodel *m)
{
	int32_t		rank;
	uint32_t	i;

	rank = 3;
	while (rank >= 1)
	{
		i = 0;
		while (i < m->napps)
		{
			if (luna_lm_match(m->apps[i].label, m->query) == rank)
				m->shown[m->nshown++] = i;
			i++;
		}
		rank--;
	}
}

static void	list_recent(t_lmodel *m)
{
	uint32_t	r;
	uint32_t	i;

	r = 0;
	while (r < m->nrecent)
	{
		i = 0;
		while (i < m->napps && m->apps[i].id != m->recent[r])
			i++;
		if (i < m->napps)
			m->shown[m->nshown++] = i;
		r++;
	}
}

static void	list_all(t_lmodel *m)
{
	uint32_t	i;
	uint32_t	j;
	uint32_t	v;

	i = 0;
	while (i < m->napps)
	{
		v = i;
		j = m->nshown;
		while (j > 0 && lp_lm_cmp(m->apps[m->shown[j - 1]].label,
				m->apps[v].label) > 0)
		{
			m->shown[j] = m->shown[j - 1];
			j--;
		}
		m->shown[j] = v;
		m->nshown++;
		i++;
	}
}

void	lp_lm_refresh(t_lmodel *m)
{
	t_lmview	v;

	m->nshown = 0;
	v = luna_lm_view(m);
	if (v == LUNA_LV_SEARCH)
		list_search(m);
	else if (v == LUNA_LV_RECENT)
		list_recent(m);
	else if (v == LUNA_LV_ALL)
		list_all(m);
	m->top = 0;
	m->sel = LUNA_LM_NONE;
	if (m->nshown > 0)
		m->sel = 0;
}

int32_t	luna_lm_move(t_lmodel *m, int32_t delta)
{
	int32_t	n;

	if (m == NULL || m->nshown == 0)
		return (LUNA_LM_NONE);
	n = (int32_t)m->nshown;
	if (m->sel < 0)
		m->sel = 0;
	else
		m->sel = lp_clamp(m->sel + delta, 0, n - 1);
	if (m->sel < m->top)
		m->top = m->sel;
	if (m->sel >= m->top + LUNA_LM_ROWS)
		m->top = m->sel - LUNA_LM_ROWS + 1;
	return (m->sel);
}

int32_t	luna_lm_scroll(t_lmodel *m, int32_t delta)
{
	int32_t	max;

	if (m == NULL)
		return (0);
	max = lp_max((int32_t)m->nshown - LUNA_LM_ROWS, 0);
	m->top = lp_clamp(m->top + delta, 0, max);
	return (m->top);
}

int32_t	luna_lm_select(t_lmodel *m, int32_t row)
{
	if (m == NULL || row < 0 || row >= LUNA_LM_ROWS
		|| m->top + row >= (int32_t)m->nshown)
		return (LUNA_LM_NONE);
	m->sel = m->top + row;
	return (m->sel);
}

int	luna_lm_selected(const t_lmodel *m, uint32_t *id)
{
	if (m == NULL || id == NULL || m->sel < 0 || m->sel >= (int32_t)m->nshown)
		return (0);
	*id = m->apps[m->shown[m->sel]].id;
	return (1);
}

const t_lmapp	*luna_lm_row(const t_lmodel *m, int32_t row)
{
	if (m == NULL || row < 0 || row >= LUNA_LM_ROWS
		|| m->top + row >= (int32_t)m->nshown)
		return (NULL);
	return (&m->apps[m->shown[m->top + row]]);
}
