#include "pm_int.h"

static int	unregister(t_pm *pm, t_pmwork *w, const char *package)
{
	int			i;
	uint32_t	k;

	if (pm_index(w->sys, w->sys_n, package) >= 0)
		return (E_PERM);
	i = pm_index(w->data, w->data_n, package);
	if (i < 0)
		return (E_NOENT);
	if (pm_path(w->reg_tmp, pm->data_root, PM_REG_NAME, ".tmp") < 0)
		return (E_INVAL);
	memcpy(w->apk, w->data[i].path, PM_PATH_MAX);
	k = (uint32_t)i;
	while (k + 1 < w->data_n)
	{
		w->data[k] = w->data[k + 1];
		k++;
	}
	w->data_n--;
	return (pm_store(pm, w));
}

int	pm_remove(t_pm *pm, const char *package)
{
	t_pmwork	*w;
	int			err;

	if (!package)
		return (E_INVAL);
	err = pm_work_open(pm, &w);
	if (err < 0)
		return (err);
	err = unregister(pm, w, package);
	if (err >= 0)
	{
		err = pm->fs->unlink(pm->ctx, w->apk);
		if (err == E_NOENT)
			err = 0;
	}
	pm_free(w);
	if (err < 0)
		return (err);
	return (0);
}
