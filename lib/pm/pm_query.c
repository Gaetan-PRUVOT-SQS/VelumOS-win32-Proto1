#include "pm_int.h"

int	pm_list(t_pm *pm, t_pmentry *out, uint32_t max)
{
	t_pmwork	*w;
	uint32_t	i;
	uint32_t	n;
	int			err;

	if (!out && max)
		return (E_INVAL);
	err = pm_work_open(pm, &w);
	if (err < 0)
		return (err);
	n = 0;
	i = 0;
	while (i < w->sys_n && n < max)
		out[n++] = w->sys[i++];
	i = 0;
	while (i < w->data_n && n < max)
	{
		if (pm_index(w->sys, w->sys_n, w->data[i].info.package) < 0)
			out[n++] = w->data[i];
		i++;
	}
	pm_free(w);
	return ((int)n);
}

int	pm_find(t_pm *pm, const char *package, t_pmentry *out)
{
	t_pmwork	*w;
	int			err;
	int			i;

	if (!package || !out)
		return (E_INVAL);
	err = pm_work_open(pm, &w);
	if (err < 0)
		return (err);
	i = pm_index(w->sys, w->sys_n, package);
	if (i >= 0)
		*out = w->sys[i];
	else
	{
		i = pm_index(w->data, w->data_n, package);
		if (i >= 0)
			*out = w->data[i];
	}
	pm_free(w);
	if (i < 0)
		return (E_NOENT);
	return (0);
}
